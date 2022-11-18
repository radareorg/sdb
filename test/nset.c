#include <sdb/sdb.h>

int main(int argc, char **argv) {
	int rc = 0;
	ut32 cas;

	Sdb *s = sdb_new (NULL, NULL, 0);
	sdb_nset (s, 32, "world", 0);

	char *res = sdb_nget (s, 32, 0);
	if (!res || strcmp (res, "world")) {
		rc = 1;
	}	
	eprintf ("hello %s\n", res);
	free (res);
	sdb_free (s);
	return rc;
}
