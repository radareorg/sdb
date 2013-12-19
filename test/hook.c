#include "sdb.h"

void ptr(void *user, const char *k, const char *v) {
	printf ("%s %s\n", k, v);
}

main() {
	Sdb *s = sdb_new (NULL, 0);
	sdb_hook (s, ptr, NULL);
	sdb_set (s, "Hello", "World", 0);
	sdb_free (s);
}
