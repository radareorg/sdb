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
		sdb_query (s, "a={\"pop\":123}");
		if (c>1) {
			sdb_query (s, "a={\"pop\":123};a:pop=1");
			if (c>2) {
				sdb_query (s, "a:pop=3");
				if (c>3) {
					sdb_query (s, "b:pop=1024");
					if (c>4) {
						sdb_query (s, "b:pop=111");
						if (c>5) {
							sdb_query (s, "b=");
						}
					}
				}
			}
		}
	}
	sdb_free (s);
	return 0;
}
