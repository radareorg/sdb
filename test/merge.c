#include <stdio.h>
#include <sdb.h>

int main() {
	const char *v;
	Sdb *s = sdb_new0 ();
	Sdb *d = sdb_new0 ();

	sdb_set (s, "key", "foo", 0);
	sdb_set (s, "key2", "bar", 0);
	sdb_set (s, "key3", "cow", 0);

	sdb_set (d, "key", "foo", 0);
	sdb_set (d, "koy2", "bar", 0);

	if (sdb_count (d) != 2) {
		fprintf (stderr, "merge: Invalid original destionation length\n");
		goto err;
	}
	bool ret = sdb_merge (d, s);
	if (ret == false) {
		fprintf (stderr, "merge: sdb_merge operation failed\n");
		goto err;
	}
	if (sdb_count (s) != 3) {
		fprintf (stderr, "merge: Invalid source length\n");
		goto err;
	}
	if (sdb_count (d) != 4) {
		fprintf (stderr, "merge: Invalid destination length\n");
		goto err;
	}
	fprintf (stderr, "sdb_merge: OK\n");
	sdb_free (s);
	sdb_free (d);
	return 0;

err:
	sdb_free (s);
	sdb_free (d);
	return 1;
}
