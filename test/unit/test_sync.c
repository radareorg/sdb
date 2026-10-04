#include "minunit.h"
#include <sdb/sdb.h>
#include <fcntl.h>
#include <sys/stat.h>

#if !__SDB_WINDOWS__
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

static void cleanup_sync_files(const char *path) {
	char file[SDB_MAX_PATH];
	unlink (path);
	snprintf (file, sizeof (file), "%s.tmp", path);
	unlink (file);
	snprintf (file, sizeof (file), "%s.journal", path);
	unlink (file);
}

static bool database_value_is(const char *path, const char *key, const char *expected) {
	Sdb *db = sdb_new (NULL, path, 0);
	if (!db) {
		return false;
	}
	const char *value = sdb_const_get (db, key, NULL);
	bool result = expected? value && !strcmp (value, expected): !value;
	sdb_free (db);
	return result;
}

static bool test_sync_reopen(void) {
	const char *path = ".test-sync-reopen.sdb";
	cleanup_sync_files (path);
	Sdb *db = sdb_new (NULL, path, 0);
	bool created = db && sdb_set (db, "key", "value", 0)
		&& sdb_set (db, "decimal", "0", 0)
		&& sdb_num_set (db, "decimal", UT64_MAX, 0)
		&& sdb_set (db, "hex", "0x0", 0)
		&& sdb_num_set (db, "hex", 0x100000001ULL, 0)
		&& sdb_sync (db);
	sdb_free (db);
	db = sdb_new (NULL, path, 0);
	bool numbers = db && sdb_num_get (db, "decimal", NULL) == UT64_MAX
		&& sdb_num_get (db, "hex", NULL) == 0x100000001ULL;
	sdb_free (db);
	bool text = database_value_is (path, "key", "value")
		&& database_value_is (path, "decimal", "18446744073709551615")
		&& database_value_is (path, "hex", "0x100000001");
	cleanup_sync_files (path);
	mu_assert_true (created, "database synchronized");
	mu_assert_true (numbers, "numeric values survive reopening");
	mu_assert_true (text, "text and numeric encodings survive reopening");
	mu_end;
}

static bool test_cdb_init_rewind(void) {
	const char *path = ".test-cdb-rewind.sdb";
	const char data[] = "database contents";
	char buf[sizeof (data)] = { 0 };
	struct cdb db = { 0 };
	db.fd = -1;
	int fd = open (path, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0600);
	if (fd == -1) {
		mu_fail ("open database file");
	}
	bool written = write (fd, data, sizeof (data)) == sizeof (data);
	bool first = seek_set (fd, 5) && cdb_init (&db, fd)
		&& cdb_read (&db, buf, sizeof (buf), 0) && !memcmp (buf, data, sizeof (data));
	memset (buf, 0, sizeof (buf));
	bool second = lseek (fd, 0, SEEK_END) != -1 && cdb_init (&db, fd)
		&& cdb_read (&db, buf, sizeof (buf), 0) && !memcmp (buf, data, sizeof (data));
	cdb_fini (&db);
	close (fd);
	unlink (path);
	mu_assert_true (written, "database contents written");
	mu_assert_true (first, "initialization rewinds the descriptor");
	mu_assert_true (second, "reinitialization rewinds the descriptor");
	mu_end;
}

static bool test_cdb_init_read_failure(void) {
	const char *path = ".test-cdb-read-failure.sdb";
	const char data[] = "database contents";
	struct cdb db = { 0 };
	db.fd = -1;
	int fd = open (path, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0600);
	if (fd == -1) {
		mu_fail ("open database file");
	}
	bool written = write (fd, data, sizeof (data)) == sizeof (data);
	bool initialized = cdb_init (&db, fd);
	bool empty = !db.map && !db.size;
	cdb_fini (&db);
	close (fd);
	unlink (path);
	mu_assert_true (written, "database contents written");
	mu_assert_false (initialized, "unreadable descriptor rejected");
	mu_assert_true (empty, "failed read leaves no partial cache");
	mu_end;
}

static bool test_sync_failure_preserves_database(void) {
#if __SDB_WINDOWS__
	mu_ignore;
#else
	const char *path = ".test-sync-failure.sdb";
	char tmp[SDB_MAX_PATH];
	struct rlimit saved_limit, failing_limit;
	struct sigaction saved_action, ignored_action;
	struct stat journal_stat;
	cleanup_sync_files (path);

	Sdb *db = sdb_new (NULL, path, 0);
	bool initial_ok = db
		&& sdb_set (db, "stable", "old", 0)
		&& sdb_sync (db)
		&& sdb_journal_open (db);
	if (!initial_ok) {
		sdb_free (db);
		cleanup_sync_files (path);
		mu_fail ("initial database setup failed");
	}
	bool updates_ok = sdb_set (db, "stable", "new", 0)
		&& sdb_set (db, "pending", "value", 0);
	if (!updates_ok || getrlimit (RLIMIT_FSIZE, &saved_limit) == -1) {
		sdb_free (db);
		cleanup_sync_files (path);
		mu_fail ("failed to prepare sync failure");
	}

	memset (&ignored_action, 0, sizeof (ignored_action));
	ignored_action.sa_handler = SIG_IGN;
	sigemptyset (&ignored_action.sa_mask);
	if (sigaction (SIGXFSZ, &ignored_action, &saved_action) == -1) {
		sdb_free (db);
		cleanup_sync_files (path);
		mu_fail ("failed to ignore SIGXFSZ");
	}
	failing_limit = saved_limit;
	failing_limit.rlim_cur = 0;
	if (setrlimit (RLIMIT_FSIZE, &failing_limit) == -1) {
		sigaction (SIGXFSZ, &saved_action, NULL);
		sdb_free (db);
		cleanup_sync_files (path);
		mu_fail ("failed to limit output size");
	}

	bool sync_result = sdb_sync (db);
	bool limit_restored = setrlimit (RLIMIT_FSIZE, &saved_limit) != -1;
	bool action_restored = sigaction (SIGXFSZ, &saved_action, NULL) != -1;
	const char *stable = sdb_const_get (db, "stable", NULL);
	const char *pending = sdb_const_get (db, "pending", NULL);
	bool memory_retained = stable && pending
		&& !strcmp (stable, "new") && !strcmp (pending, "value");
	bool journal_retained = fstat (db->journal, &journal_stat) != -1
		&& journal_stat.st_size > 0;
	bool disk_preserved = database_value_is (path, "stable", "old")
		&& database_value_is (path, "pending", NULL);
	snprintf (tmp, sizeof (tmp), "%s.tmp", path);
	bool temp_removed = access (tmp, F_OK) == -1;

	bool retry_result = sdb_sync (db);
	bool disk_updated = database_value_is (path, "stable", "new")
		&& database_value_is (path, "pending", "value");
	bool journal_cleared = fstat (db->journal, &journal_stat) != -1
		&& journal_stat.st_size == 0;

	sdb_free (db);
	cleanup_sync_files (path);
	mu_assert_false (sync_result, "failed sync result");
	mu_assert_true (limit_restored, "restore file-size limit");
	mu_assert_true (action_restored, "restore SIGXFSZ handler");
	mu_assert_true (memory_retained, "pending changes retained in memory");
	mu_assert_true (journal_retained, "journal retained after failure");
	mu_assert_true (disk_preserved, "existing database preserved");
	mu_assert_true (temp_removed, "failed temporary database removed");
	mu_assert_true (retry_result, "retry succeeds");
	mu_assert_true (disk_updated, "retry persists pending changes");
	mu_assert_true (journal_cleared, "journal cleared after success");
	mu_end;
#endif
}

static bool test_unlink_removes_database(void) {
#if __SDB_WINDOWS__
	mu_ignore;
#else
	const char *path = ".test-unlink.sdb";
	cleanup_sync_files (path);
	Sdb *db = sdb_new (NULL, path, 0);
	bool created = db && sdb_set (db, "key", "value", 0) && sdb_sync (db);
	sdb_free (db);
	if (!created) {
		cleanup_sync_files (path);
		mu_fail ("initial database setup failed");
	}
	// reopen so the reader fd and the mapping are live when unlinking
	db = sdb_new (NULL, path, 0);
	bool loaded = db && sdb_const_get (db, "key", NULL) != NULL;
	bool unlinked = loaded && sdb_unlink (db);
	sdb_gh_free (db); // sdb_unlink finalizes the struct but does not free it
	bool removed = access (path, F_OK) == -1;
	bool stdin_alive = fcntl (0, F_GETFD) != -1;
	cleanup_sync_files (path);
	mu_assert_true (loaded, "database reopened from disk");
	mu_assert_true (unlinked, "sdb_unlink result");
	mu_assert_true (removed, "database file removed");
	mu_assert_true (stdin_alive, "stdin untouched by unlink");
	mu_end;
#endif
}

static int all_tests(void) {
	mu_run_test (test_sync_reopen);
	mu_run_test (test_cdb_init_rewind);
	mu_run_test (test_cdb_init_read_failure);
	mu_run_test (test_sync_failure_preserves_database);
	mu_run_test (test_unlink_removes_database);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
