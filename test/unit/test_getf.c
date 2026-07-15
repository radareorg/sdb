#include "minunit.h"
#include <sdb/sdb.h>

static bool test_getf(void) {
	Sdb *db = sdb_new0 ();
	mu_assert_notnull (db, "database");
	mu_assert ("set", sdb_set (db, "user.alice.42", "admin", 0));
	mu_assert ("formatted set", sdb_setf (db, "staff", 0, "group.%s.%d", "alice", 42));
	mu_assert_streq (sdb_const_get (db, "group.alice.42", NULL), "staff", "formatted set value");
	mu_assert ("formatted number set", sdb_num_setf (db, 42, 0, "number.%s", "answer"));
	mu_assert_eq (sdb_num_get (db, "number.answer", NULL), 42, "formatted number set value");

	ut32 const_cas = 0;
	const char *const_value = sdb_const_getf (db, &const_cas,
		"user.%s.%d", "alice", 42);
	mu_assert_streq (const_value, "admin", "formatted const get");
	mu_assert ("const get CAS", const_cas != 0);

	char maxkey[SDB_MAX_KEY];
	memset (maxkey, 'x', sizeof (maxkey) - 1);
	maxkey[sizeof (maxkey) - 1] = 0;
	mu_assert ("set maximum key", sdb_set (db, maxkey, "maximum", 0));
	mu_assert_streq (sdb_const_getf (db, NULL, "%s", maxkey), "maximum",
		"maximum formatted key");

	char oversized[SDB_MAX_KEY + 1];
	memset (oversized, 'x', sizeof (oversized) - 1);
	oversized[sizeof (oversized) - 1] = 0;
	ut32 cas = 123;
	mu_assert_null (sdb_const_getf (db, &cas, "%s", oversized),
		"oversized formatted key");
	mu_assert_eq (cas, 0, "oversized key CAS");
	mu_assert_null (sdb_const_getf (db, NULL, "missing.%d", 7),
		"missing formatted key");

	sdb_free (db);
	mu_end;
}

int all_tests(void) {
	mu_run_test (test_getf);
	return tests_passed != tests_run;
}

int main(void) {
	return all_tests ();
}
