#include <sdb.h>

int main(int argc, char **argv) {
	int i, N;
	if (argc>1)
		N = atoi (argv[1]);
	else N = 1000;
	Sdb *s = sdb_new (NULL, NULL, 0);
	for (i=0;i<N;i++)
		sdb_array_push (s, "key", "foo", 0);
	for (i=0;i<N;i++)
		free (sdb_array_pop (s, "key", 0));
	sdb_free (s);
	return 0;
}
