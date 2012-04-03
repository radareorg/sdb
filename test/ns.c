#include <sdb.h>

int main () {
	Sdb *s = sdb_new ("foo", 0);
// TODO: unlink bar file"
	Sdb *n = sdb_ns (s, "bar");
	sdb_set (n, "user.pancake", "pancake foo", 0);
	sdb_set (n, "user.password", "jklsdf8r3o", 0);
	sdb_ns_sync (s, NULL);
// if "bar" file exists = WIN //
	return 0;
}
