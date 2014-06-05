#include "sdb.h"

int main () {
	//Sdb *s = sdb_new (NULL, "/tmp/", 1);
	Sdb *s = sdb_new0 ();
	if (!s) {
		eprintf ("SDB FAIL\n");
	}
// TODO: unlink bar file"
	Sdb *n = sdb_ns (s, "bar", 1);
	if (n == NULL) {
		eprintf ("n = NULL!\n");
		return 1;
	}
	sdb_set (n, "user.pancake", "pancake foo", 0);
	sdb_set (n, "user.password", "jklsdf8r3o", 0);
	sdb_ns_sync (s);
// if "bar" file exists = WIN //
	return 0;
}
