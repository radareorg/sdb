#include "minunit.h"
#include <sdb/sdb.h>
#include <fcntl.h>
#include <sys/stat.h>

#if !__SDB_WINDOWS__
#include <signal.h>
#include <sys/resource.h>

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
#endif

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

static int all_tests(void) {
	mu_run_test (test_sync_failure_preserves_database);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
