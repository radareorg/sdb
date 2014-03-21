#include <sdb.h>

// test-invalidate-disk
int main() {
	const char *res;
	Sdb *s = sdb_new (NULL, "db.test", 0);
	res = sdb_const_get (s, "foo", 0);
	if (!res) {
		printf ("Oops. db.test database is not initialized\n");
		return 1;
	}
	printf ("%s\n", res);
	sdb_set (s, "foo", "new", 0);
	printf ("%s\n", sdb_const_get (s, "foo", 0));
	sdb_reset (s);
	res = sdb_const_get (s, "foo", 0);
	printf ("%p\n", res);
	if (res) {
		printf ("\nOops. sdb_reset didnt ignored the disk key\n");
	} else printf ("\nIt Works!\n");
	//	sdb_sync (s);
	sdb_free (s);
	return (res)? 1: 0;
}
