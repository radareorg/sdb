#include <sdb/sdb.h>

int main(int argc, char **argv) {
	int rc = 0;
	ut32 cas = 0, cas2 = 0;

	Sdb *s = sdb_new (NULL, NULL, 0);
	if (!sdb_set (s, "hello", "world", 0)) {
		printf ("error: initial set failed\n");
		rc = 1;
	}
	sdb_const_get (s, "hello", &cas);
	if (!cas) {
		printf ("error: no cas after set\n");
		rc = 1;
	}
	if (!sdb_set (s, "hello", "mundo", cas)) {
		printf ("error: set with matching cas failed\n");
		rc = 1;
	}
	sdb_const_get (s, "hello", &cas2);
	if (cas2 == cas) {
		printf ("error: cas did not change after write\n");
		rc = 1;
	}
	if (sdb_set (s, "hello", "monde", cas)) {
		printf ("error: set with stale cas succeeded\n");
		rc = 1;
	}
	printf (rc? "error\n": "  ok\n");
	sdb_free (s);
	return rc;
}
