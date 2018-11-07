/* radare - LGPL - Copyright 2009-2018 pancake */

#include "./r_pair.h"

void test_pair ();

struct item_t {
	char city[10];
	int people;
	int id;
};

#define K_ID R_DB_INDEXOF(struct item_t, id)
#define K_CITY R_DB_INDEXOF(struct item_t, city)

int main(int argc, char **argv) {
	test_pair ();
	return 0;
}

void test_pair () {
	char *s;
	RPair *p = r_pair_new ();
	r_pair_set_sync_dir (p, "sdb-db-test");

	r_pair_set (p, "root", "clown");
	r_pair_set (p, ".branch", "clown");
	r_pair_set (p, "user.name", "pancake");
	r_pair_set (p, "user.pass", "password");

	s = r_pair_get (p, "user.name");
	printf ("user.name=%s\n", s);

	{
	RPairItem *o;
	RListIter *iter;
	RList *list = r_pair_list (p, "user");
	printf ("DUMP:\n");
	r_list_foreach (list, iter, o) {
		printf ("   %s = %s\n", o->k, o->v);
	}
	r_list_purge (list);
	}
	r_pair_sync (p);
}

