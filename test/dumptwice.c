#include <sdb.h>

#define DBFILE "dumptwice.db"

int main() {
	int ret = 1;
	Sdb *sdb;
	eprintf ("[i] Running dumptwice test\n");
	unlink (DBFILE);
	sdb = sdb_new ("./", DBFILE, 0);
	sdb_set (sdb, "foo", "bar", 0);
	sdb_sync (sdb);
	sdb_set (sdb, "foo", "win", 0);
	sdb_sync (sdb);
	sdb_free (sdb);
	sdb = sdb_new ("./", DBFILE, 0);
	if (!strcmp (sdb_const_get (sdb, "foo", 0), "win")) {
		ret = 0;
	}
	sdb_free (sdb);
	unlink (DBFILE);
	return ret;
}
