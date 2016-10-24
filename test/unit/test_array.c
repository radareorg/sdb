#include "minunit.h"
#include <sdb.h>

bool test_sdb_array(void) {
	Sdb *db = sdb_new (NULL, NULL, false);
	sdb_array_push (db, "foo", "foo", 0);
	sdb_array_push (db, "foo", "bar", 0);
	sdb_array_push (db, "foo", "cow", 0);

	mu_assert_streq ("cow,bar,foo", sdb_const_get (db, "foo", 0), "Item not deleted");

	sdb_free (db);
	mu_end;
}

bool test_sdb_array_add(void) {
	Sdb *db = sdb_new (NULL, NULL, false);
	sdb_array_add (db, "foo", "foo", 0);
	sdb_array_add (db, "foo", "bar", 0);
	sdb_array_add (db, "foo", "cow", 0);

	mu_assert_streq ("foo,bar,cow", sdb_const_get (db, "foo", 0), "Item not deleted");

	sdb_free (db);
	mu_end;
}


int all_tests() {
	mu_run_test (test_sdb_array);
	mu_run_test (test_sdb_array_add);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
