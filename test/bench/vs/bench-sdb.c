#include <sdb.h>

int main() {
	int i;
	Sdb *s = sdb_new (".","___test.sdb",0);
	for (i=0;i<100000; i++) {
		sdb_set (s, sdb_fmt(0, "foo%d", i), "bar", 0);
	}
	sdb_sync (s);
	sdb_free (s);
}
