#include <sdb/sdb.h>

int main() {
#define DBFILE "___syncget.db"
	const char *v;
	Sdb *s = sdb_new0 ();
	unlink (DBFILE);
	sdb_set (s, "foo", "bar", 0);
	char *val = sdb_get (s, "foo", NULL);
	eprintf ("-> %s\n", val);
	free (val);
	sdb_file (s, DBFILE);
	sdb_sync (s);
	v = sdb_const_get (s, "foo", NULL);
	if (v && !strcmp ("bar",  v)) {
		eprintf ("OK syncget\n");
	} else {
		eprintf ("ERROR syncget: Keys not accessible after sync\n");
		sdb_free (s);
		unlink (DBFILE);
		return 1;
	}
	sdb_unset (s, "foo", 0);
	v = sdb_const_get (s, "foo", NULL);
	if (v) {
		eprintf ("ERROR syncget: Unset key still accessible before sync\n");
		sdb_free (s);
		unlink (DBFILE);
		return 1;
	}
	sdb_sync (s);
	sdb_free (s);
	s = sdb_new (".", DBFILE, 0);
	v = sdb_const_get (s, "foo", NULL);
	if (v) {
		eprintf ("ERROR syncget: Unset key still accessible after reopen\n");
		sdb_free (s);
		unlink (DBFILE);
		return 1;
	}
	sdb_free (s);
	unlink (DBFILE);
	return 0;
}
