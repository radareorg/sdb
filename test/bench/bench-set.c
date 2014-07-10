#include <sdb.h>
#include "prof.c"

void dosync (int count) {
	char rkey[128];
	RProfile p;
	int i;
	Sdb *db = sdb_new (NULL, NULL, 0);
	r_prof_start (&p);
	for (i=0; i<count; i++) {
		sprintf (rkey, "%d", i);
		sdb_set (db, rkey, rkey, 0);
	}
	r_prof_end (&p);
	sdb_free (db);
	printf (__FILE__" %lf %d\n", p.result, i);
}

int main(int argc, char **argv) {
	dosync (100);
	dosync (1000);
	dosync (10000);
	dosync (100000);
	dosync (1000000);
#if 0
	dosync (10000000);
	dosync (100000000);
#endif
	return 0;
}
