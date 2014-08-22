#include "sdb.h"

int main () {
	//Sdb *s = sdb_new (NULL, "/tmp/", 1);
	Sdb *s = sdb_new0 ();
	if (!s) {
		eprintf ("SDB FAIL\n");
	}
#if 1
	Sdb *n = sdb_ns (s, "bar", 1);
	sdb_set (n, "user.pancake", "pancake foo", 0);
	sdb_set (n, "user.password", "jklsdf8r3o", 0);
	sdb_ns_set (s, "food", n);
	sdb_ns_set (sdb_ns (s, "food", 1), "penis", n); //sdb_new0());
//	sdb_ns_set (sdb_ns (s, "boom", 1), "penis", n); //sdb_new0());
//	sdb_ns_set (sdb_ns (s, "faty", 1), "penis", n); //sdb_new0());
#else
// TODO: unlink bar file"
	Sdb *n = sdb_ns (s, "bar", 1);
	if (n == NULL) {
		eprintf ("n = NULL!\n");
		return 1;
	}
	sdb_set (n, "user.pancake", "pancake foo", 0);
	sdb_set (n, "user.password", "jklsdf8r3o", 0);
	sdb_ns_set (s, "food", n);

	sdb_ns_set (sdb_ns (s, "food", 1), "penis", n); //sdb_new0());
	sdb_ns_set (sdb_ns (s, "food", 1), "ponis", n); //sdb_new0());
	sdb_ns_set (sdb_ns (s, "food", 1), "powns", n); //sdb_new0());
	//sdb_ns (sdb_ns (s, "food"), "penis");
//	sdb_ns_sync (s);
#endif
// if "bar" file exists = WIN //
	sdb_free (s);
	return 0;
}
