#include <sdb.h>

int main(int argc, char **argv) {
	int r;
	ut32 cas;
	Sdb *s = sdb_new (NULL, 0);
	r = sdb_set (s, "hello", "world", 1);
	sdb_getc (s, "hello", &cas);
	printf ("r%d = c%u\n", r, cas);
	if (r != cas) printf ("error\n");

	r = sdb_set (s, "hello", "world", r);
	sdb_getc (s, "hello", &cas);
	if (r != cas) printf ("error\n");

	printf ("r%d = c%u\n", r, cas);
	if (r==0) printf ("failed\n");
	sdb_free (s);
	return 0;
}
