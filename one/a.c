#include "sdb-one.c"

int main() {
	printf ("Testing foo\n");
	Sdb *s = sdb_new (NULL, NULL, 0);
	sdb_set (s, "Hello", "World", 0);
sdb_array_push (s, "Hello", ":D",0);
	printf ("Hello %s\n", sdb_const_get (s, "Hello", 0));
	sdb_free (s);
	return 0;
}
