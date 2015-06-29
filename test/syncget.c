#include <sdb.h>

int main() {
#define DBFILE "___syncget.db"
	const char *v;
	Sdb *s = sdb_new0();
	unlink (DBFILE);
	sdb_set (s, "foo", "bar", 0);
	eprintf ("-> %s\n", sdb_get (s, "foo", NULL));
	sdb_file (s, DBFILE);
	sdb_sync (s);
	v = sdb_const_get (s, "foo", NULL);
	if (v && !strcmp ("bar",  v)) {
		eprintf ("OK syncget\n");
		return 0;
	}
	eprintf ("ERROR syncget: Keys not accessible after sync\n");
	return 1;
}
