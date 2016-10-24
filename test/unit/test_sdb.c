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

bool test_sdb_delete_none(void) {
	Sdb *db = sdb_new (NULL, NULL, false);
	sdb_set (db, "foo", "bar", 0);
	sdb_set (db, "bar", "cow", 0);
	sdb_set (db, "low", "bar", 0);
	sdb_unset (db, "fundas", 0);
	sdb_unset (db, "barnas", 0);
	sdb_unset (db, "bar", 0);
	sdb_unset (db, "pinuts", 0);
	SdbList *list = sdb_foreach_list (db);
	mu_assert_eq ((int)ls_length (sdb_foreach_list (db)), 2, "Unmatched rows");
	sdb_free (db);
	mu_end;
}

bool test_sdb_delete_alot(void) {
	char key[128];
	Sdb *db = sdb_new (NULL, NULL, false);
	const int count = 2048;
	int i;
	for (i = 0; i < count; i++) {
		sdb_set (db, sdb_fmt (0, "key.%d", i), "bar", 0);
	}
	for (i = 0; i < count; i++) {
		sdb_unset (db, sdb_fmt (0, "key.%d", i), 0);
	}
	SdbList *list = sdb_foreach_list (db);
	mu_assert_eq ((int)ls_length (sdb_foreach_list (db)), 0, "Unmatched rows");
	sdb_free (db);
	mu_end;
}

int all_tests() {
	mu_run_test (test_sdb_foreach_delete);
	mu_run_test (test_sdb_list_delete);
	mu_run_test (test_sdb_delete_none);
	mu_run_test (test_sdb_delete_alot);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
