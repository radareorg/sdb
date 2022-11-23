#include <sdb/sdb.h>

int main() {
	int i;
	char foo[32];
	Sdb *s = sdb_new (".","___test.sdb",0);
	for (i=0;i<100000; i++) {
		snprintf (foo, sizeof (foo), "foo%d", i);
		sdb_set (s, foo, "bar", 0);
	}
	sdb_sync (s);
	sdb_free (s);
	return 0;
}
