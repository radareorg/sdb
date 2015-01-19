#include <sdb.h>
#include <assert.h>

int main() {
	Sdb *s = sdb_new0 ();
	sdb_set (s, "key", "val", 0);
	sdb_expire_set (s, "key", 3, 0);
	sleep (1);
	assert (sdb_const_get (s, "key", 0) != NULL);
	sleep (3);
	assert (sdb_const_get (s, "key", 0) == NULL);
	sdb_free (s);
	return 0;
}
