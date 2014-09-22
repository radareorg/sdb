#include <sdb.h>

int main() {
	char *cur;
	sdb_aforeach (cur, strdup ("foo,bar,cow")) {
		printf ("--> %s\n", cur);
		sdb_aforeach_next (cur);
	}
	return 0;
}
