#include <sdb.h>

int main() {
	char *cur, *csv = strdup ("foo,bar,cow");
	sdb_aforeach (cur, csv) {
		eprintf ("--> %s\n", cur);
		sdb_aforeach_next (cur);
	}
	eprintf ("(%s)\n", csv);
	free (csv);
	return 0;
}
