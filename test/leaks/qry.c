#include "sdb.h"

int main (int argc, char **argv) {
	//Sdb *s = sdb_new (NULL, "/tmp/", 1);
	int c = argc>1?atoi (argv[1]):0;
	Sdb *s = sdb_new0 ();
	if (!s) {
		eprintf ("SDB FAIL\n");
		return 1;
	}
	if (c>0) {
		sdb_query (s, "a=b");
		if (c>1) {
			sdb_query (s, "a");
			if (c>2) {
				sdb_query (s, "b=c");
			}
		}
	}
	sdb_free (s);
	return 0;
}
