#include <sdb.h>

int main(int argc, char **argv) {
	int r, rc = 0;
	ut32 cas;

	Sdb *s = sdb_new (NULL, NULL, 0);
	r = sdb_set (s, "hello", "world", 1);
	sdb_const_get (s, "hello", &cas);
	printf ("[test] r%d = c%u\n", r, cas);
	if (r != cas) {
		printf ("error\n");
		rc = 1;
	} else {
		printf ("  ok\n");
	}

	r = sdb_set (s, "hello", "world", r);
	sdb_const_get (s, "hello", &cas);
	printf ("[test] r%d = c%u\n", r, cas);
	if (r != cas) {
		printf ("error\n");
		rc = 1;
	} else {
		printf ("  ok\n");
	}
	printf ("[test] r%d = c%u\n", r, cas);
	if (r == 0) {
		printf ("error\n");
		rc = 1;
	} else {
		printf ("  ok\n");
	}
	sdb_free (s);
	return rc;
}
