#include <sdb-one.h>

int main(int argc, char **argv) {
	HtPP *h = ht_new (free);
	char k[128];
	int i, len = atoi (argv[1]);

	for (i = 0; i < len; i++) {
		sprintf (k, "keynum%d", i);
		ht_insert (h, sdb_hash(k, 0), strdup (
			"jkladjfklafjklsajfklsajdflksadjfklsadjflk"
			), NULL);
	}
	printf ("JGAKJK\n");
	sleep (11111);
	ht_free (h);
}

#if 0
int main(int argc, char **argv) {
	Sdb *s = sdb_new (NULL, NULL, 0); //"____", 0);
	int len = atoi (argv[1]);
	int i = 0; 
	char k[32];
// 1s
// 3.2s
// 2.7
	for (i=0;i<len; i++) {
		sprintf (k, "keynum%d", i);
		sdb_set (s, k, "jkladjfklafjklsajfklsajdflksadjfklsadjflk", 0);
	}
printf ("TAKE MY MORMORE\n");
#if 0
//sleep (1000);
// ~0
	for (i=0;i<len; i++) {
		//sprintf (k, "keynum%d", i);
		const char *p = sdb_const_get (s, "keynum33", 0);//, "jkladjfklafjklsajfklsajdflksadjfklsadjflk", 0);
k[0] = p[0];
	}
// 800 ms - if no db
#endif

// 2.8 s  - if db exist
	//sdb_sync (s);
// 200 ms
	sdb_free (s);
	return 0;
}
#endif
