#include <sdb.h>

int main ()
{
	char s[64];
	int ret;
	Sdb *db = sdb_new0();
	sdb_itoa (3, s, 10);
	sdb_num_set (db, s, 3, 0);
	ret = !sdb_num_exists (db, s);
	sdb_free (db);
	return ret;
}
