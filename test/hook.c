#include "sdb.h"

static void ptr(Sdb *s, void *user, const char *k, const char *v) {
	printf ("%s %s\n", k, v);
}

int main() {
	Sdb *s = sdb_new (NULL, NULL, 0);
	sdb_hook (s, ptr, NULL);
	sdb_set (s, "Hello", "World", 0);
	sdb_free (s);
	return 0;
}
