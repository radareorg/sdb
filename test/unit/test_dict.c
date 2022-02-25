#include <sdb.h>

int main() {
	dict m;
	dict_init (&m, 2, free);
	dict_set (&m, 0x100, 1, NULL);
	dict_set (&m, 0x200, 2, NULL);
	dict_set (&m, 0x300, 3, NULL);
	dict_set (&m, 0x400, 4, NULL);
	printf ("%d %d\n", (int)dict_get (&m, 0x100), (int)dict_get (&m, 0x200));
	printf ("%d %d\n", (int)dict_get (&m, 0x300), (int)dict_get (&m, 0x400));
	if ((int)dict_get (&m, 0x100) != 1) {
		return 1;
	}
	if ((int)dict_get (&m, 0x200) != 2) {
		return 1;
	}
	if ((int)dict_get (&m, 0x300) != 3) {
		return 1;
	}
	if ((int)dict_get (&m, 0x400) != 4) {
		return 1;
	}

	int buckets = dict_stats (&m, -1);
	int i;
	for (i = 0; i < buckets; i++) {
		printf ("%d: %d\n", i, dict_stats (&m, i));
	}
#if 0
	dict_set(&m, dict_hash("username"), 1024, NULL);
	dict_set(&m, 32, 212, strdup("test"));
	dict_del(&m, dict_hash("username"));
	printf ("%d\n", (int)dict_get(&m, dict_hash("username")));
	printf ("%s\n", dict_getu(&m, 32)); //dict_hash("username")));
#endif
	dict_fini (&m);
	return 0;
}
