#include <sdb.h>

int main(int argc, char **argv) {
	Sdb *s = sdb_new (NULL, "____", 0);
	int len = atoi (argv[1]);
	int i = 0; 
	char k[32];
// 1s
	for (i=0;i<len; i++) {
		sprintf (k, "keynum%d", i);
		sdb_set (s, k, "jkladjfklafjklsajfklsajdflksadjfklsadjflk", 0);
	}
// 800 ms - if no db
// 2.8 s  - if db exist
	sdb_sync (s);
// 200 ms
	sdb_free (s);
	return 0;
}
