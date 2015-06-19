#include <sdb.h>

#define DBFILE "dumptwice.db"

int main() {
	int ret = 1;
	Sdb *sdb;
	const char *res;

	eprintf ("\033[33m[i] Running dumptwice test:  \033[0m");
	unlink (DBFILE);
	sdb = sdb_new (".", DBFILE, 0);
	sdb_set (sdb, "foo", "bar", 0);
	sdb_sync (sdb);
	sdb_set (sdb, "foo", "win", 0);
	sdb_sync (sdb);
	sdb_free (sdb);
	sdb = sdb_new (".", DBFILE, 0);

	res = sdb_const_get (sdb, "foo", 0);
	if (res && !strcmp (res, "win")) {
		ret = 0;
	}
	if (ret) {
		eprintf ("\033[31mERROR\033[0m\n");
	} else {
		eprintf ("\033[32mOK\033[0m\n");
	}
	sdb_free (sdb);
	unlink (DBFILE);
	return ret;
}
