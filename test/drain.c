#include <stdio.h>
#include <sdb.h>

int main() {
	const char *v;
	Sdb *s = sdb_new0 ();
	sdb_set (s, "key", "foo", 0);
	v = sdb_const_get (s, "key", 0);
	printf ("%p %p (assert != 0)\n", s, v);
	if (!v) return 1;

	sdb_drain (s, sdb_new0 ());
	v = sdb_const_get (s, "key", 0);
	printf ("%p %p (assert == 0)\n", s, v);
	sdb_free (s);
	return (v==0)?0:1;
}
