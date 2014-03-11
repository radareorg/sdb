#include <sdb.h>
main(int argc, char **argv) {
	Sdb *s = sdb_new (NULL, "____", 0);
	int len = atoi (argv[1]);
	int i = 0; 
	char k[32];
	for (i=0;i<len; i++) {
		sprintf (k, "keynum%d", i);
		sdb_set (s, k, "jkladjfklafjklsajfklsajdflksadjfklsadjflk", 0);
	}
	sdb_sync (s);
	sdb_free (s);
}
