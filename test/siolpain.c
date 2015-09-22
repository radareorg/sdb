#include <sdb.h>

int main () {
	char s[64];
	int ret;
	Sdb *db = sdb_new0 ();
	s[0] = 0;
	sdb_itoa (3, s, 10);
	eprintf ("IOTA: %s\n", s);
	sdb_num_set (db, s, 3, 0);
	ret = !sdb_num_exists (db, s);
	sdb_free (db);
	return ret;
}
