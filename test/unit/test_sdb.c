#include "minunit.h"
#include <sdb.h>

static int foreach_delete_cb(void *user, const char *key, const char *val) {
	if (strcmp (key, "bar")) {
		sdb_unset (user, key, 0);
	}
	return 1;
}

bool test_sdb_foreach_delete(void) {
	Sdb *db = sdb_new (NULL, NULL, false);
	sdb_set (db, "foo", "bar", 0);
	sdb_set (db, "bar", "cow", 0);
	sdb_set (db, "low", "bar", 0);
	sdb_foreach (db, foreach_delete_cb, db);

	mu_assert ("Item not deleted", !(int)(size_t)sdb_const_get (db, "foo", NULL));
	mu_assert ("Item not deleted", (int)(size_t)sdb_const_get (db, "bar", NULL));

	sdb_free (db);
	mu_end;
}

bool test_sdb_list_delete(void) {
	Sdb *db = sdb_new (NULL, NULL, false);
	sdb_set (db, "foo", "bar", 0);
	sdb_set (db, "bar", "cow", 0);
	sdb_set (db, "low", "bar", 0);
	SdbList *list = sdb_foreach_list (db);
	SdbListIter *iter;
	SdbKv *kv;
	ls_foreach (list, iter, kv) {
		//printf ("--> %s\n", kv->key);
		sdb_unset (db, kv->key, 0);
	}
	ls_free (list);
	mu_assert ("List is empty", !ls_length (sdb_foreach_list (db)));
	sdb_free (db);
	mu_end;
}

int all_tests() {
	mu_run_test (test_sdb_foreach_delete);
	mu_run_test (test_sdb_list_delete);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
