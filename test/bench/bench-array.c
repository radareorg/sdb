#include <sdb.h>
#include "prof.c"

void arradd (int count) {
	char rkey[128];
	RProfile p;
	int i;
	Sdb *db = sdb_new (NULL, NULL, 0);
	r_prof_start (&p);
	for (i=0; i<count; i++) {
		sprintf (rkey, "%d", i);
		sdb_array_set (db, "foo", -1, rkey, 0);
	}
	r_prof_end (&p);
	sdb_free (db);
	printf (__FILE__" %lf %d\n", p.result, i);
}

int main(int argc, char **argv) {
	arradd (100);
	arradd (1000);
	arradd (10000);
	arradd (17000);
#if 0
	arradd (40000);
	arradd (100000);
	// ETOOSLOW //
	arradd (10000000);
	arradd (100000000);
#endif
	return 0;
}
