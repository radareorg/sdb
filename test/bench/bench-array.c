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
	printf (__FILE__" add\t\t\t %lf %d\n", p.result, i);
}

void arrsort (int count) {
	char rkey[128];
	RProfile p;
	int i;
	Sdb *db = sdb_new (NULL, NULL, 0);
	int s = 0x27012701;
	for (i=0; i<count; i++) {
		s ^= s >> 13;
		s ^= s << 18;
		sprintf (rkey, "%d", s & 0x7fffffff);
		sdb_array_set (db, "foo", -1, rkey, 0);
	}
	r_prof_start (&p);
	sdb_array_sort (db, "foo", 0);
	r_prof_end (&p);
	printf (__FILE__" sort\t\t\t %lf %d\n", p.result, i);
	r_prof_start (&p);
	sdb_array_sort (db, "foo", 0);
	r_prof_end (&p);
	printf (__FILE__" sort (sorted)\t\t %lf %d\n", p.result, i);
	sdb_free (db);
}

void arrsort_num (int count) {
	char rkey[128];
	RProfile p;
	int i;
	Sdb *db = sdb_new (NULL, NULL, 0);
	int s = 0x2701;
	for (i=0; i<count; i++) {
		s ^= s >> 13;
		s ^= s << 18;
		sprintf (rkey, "%d", s & 0x7fffffff);
		sdb_array_set (db, "foo", -1, rkey, 0);
	}
	r_prof_start (&p);
	sdb_array_sort_num (db, "foo", 0);
	r_prof_end (&p);
	printf (__FILE__" numeric sort\t\t %lf %d\n", p.result, i);
	r_prof_start (&p);
	sdb_array_sort_num (db, "foo", 0);
	r_prof_end (&p);
	printf (__FILE__" numeric sort (sorted)\t %lf %d\n", p.result, i);
	sdb_free (db);
	db = sdb_new (NULL, NULL, 0);
	for (i=count; i>0; i--) {
		sprintf(rkey, "%d", i);
		sdb_array_set (db, "foo", -1, rkey, 0);
	}
	r_prof_start (&p);
	sdb_array_sort_num (db, "foo", 0);
	r_prof_end (&p);
	printf (__FILE__" numeric sort (reverse)\t %lf %d\n", p.result, count);
	sdb_free (db);
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
	arrsort (100);
	arrsort (1000);
	arrsort (10000);
	arrsort_num (100);
	arrsort_num (1000);
	arrsort_num (10000);
	return 0;
}
