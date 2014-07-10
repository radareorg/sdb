#include <sdb.h>
#include "prof.c"


void construct (int count) {
	RProfile p;
	int i;
	r_prof_start (&p);
	for (i=0; i<count; i++) {
		Sdb *db = sdb_new0 ();
		/* do nothing */
		sdb_free (db);
	}
	r_prof_end (&p);
	printf (__FILE__" %lf %d\n", p.result, i);
}

int main(int argc, char **argv) {
	construct (100);
	construct (1000);
	construct (10000);
	construct (100000);
	construct (1000000);
#if 0
	construct (10000000);
	construct (100000000);
#endif
	return 0;
}
