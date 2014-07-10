#include <sdb.h>
#include "prof.c"

void arradd (int count) {
	RProfile p;
	int i;
	Sdb *db = sdb_new (NULL, NULL, 0);
	r_prof_start (&p);
	for (i=0; i<count; i++) {
		sdb_array_push (db, "foo", "bar", 0);
	}
	for (i=0; i<count; i++) {
		free (sdb_array_pop (db, "foo", 0));
	}
	if (sdb_const_get (db, "foo", 0) != NULL) {
		eprintf (__FILE__" ASSERT stack not empty\n");
	}
	// TODO: verify array is empty
	r_prof_end (&p);
	sdb_free (db);
	printf (__FILE__" %lf %d\n", p.result, i);
}

int main(int argc, char **argv) {
	arradd (100);
	arradd (1000);
	arradd (10000);
	arradd (20000);
	arradd (40000);
#if 0
	// ETOOSLOW //
	arradd (100000);
	arradd (1000000);
	arradd (10000000);
	arradd (100000000);
#endif
	return 0;
}
