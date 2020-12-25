#include <sdb.h>

int main() {
	int rc = 1;
	Sdb *db = sdb_new0 ();
	Sdb *a = sdb_new0 ();
	sdb_set (a, "hello", "world", 0);
	sdb_ns_set (db, "foo", a);
	Sdb *b = sdb_ns (db, "bar", 1);
	sdb_set (b, "hello", "world", 0);
	SdbNs *ns;
	SdbListIter *it;
	int oks = 0;
	ls_foreach (db->ns, it, ns) {
		eprintf ("%s\n", ns->name);
		char *o = sdb_querysf (ns->sdb, NULL, 0, "*");
		if (o) {
			eprintf ("%s\n", o);
			if (!strcmp (o, "hello=world\n")) {
				eprintf ("Works\n");
				oks++;
			}
		}
	}
	if (oks == 2) {
		rc = 0;
	}
	sdb_free (db);
	return rc;
}
