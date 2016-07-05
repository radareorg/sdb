#include <sdb.h>

#define MAX 19999999

int main() {
	int i;
	Sdb *s = sdb_new0 ();
	sdb_set (s, "foo", "bar", 0);
	for (i = 0; i < MAX ; i++) {
		if (sdb_set (s, "foo", "bar", 0)) {
			fprintf (stderr, "milset: sdb_set failed\n");
			return 1;
		}
	}
	sdb_free (s);
	return 0;
}
