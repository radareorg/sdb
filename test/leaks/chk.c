#include "sdb.h"

int main (int argc, char **argv) {
	//Sdb *s = sdb_new (NULL, "/tmp/", 1);
	int c = argc>1?atoi (argv[1]):0;
	Sdb *s = sdb_new0 ();
	if (!s) {
		eprintf ("SDB FAIL\n");
	}
	if (c>0) {
		Sdb *n = sdb_ns (s, "bar", 1);
		if (c>1) {
			sdb_set (n, "user.pancake", "pancake foo", 0);
		}
		if (c>2) {
			sdb_set (n, "user.password", "jklsdf8r3o", 0);
		}
		if (c>3)
			sdb_ns_set (s, "food", n);
		if (c>4)
			sdb_ns_set (sdb_ns (s, "food", 1), "penis", n);
		if (c>5)
			sdb_ns_set (sdb_ns (s, "flood", 1), "penis", n);
	}
	sdb_free (s);
	return 0;
}
