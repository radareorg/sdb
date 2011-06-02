#include "sdb.h"
#include <ctype.h>

static void cmdhash(const char *s) {
	int i;
	char su[100];
	for (i=0; s[i]; i++)
		su[i] = toupper (s[i]);
	su[i] = 0;
	printf ("#define MCSDB_CMD_%s 0x%x\n", su, sdb_hash (s));
}

int main() {
	cmdhash ("version");
	cmdhash ("append");
	cmdhash ("prepend");
	cmdhash ("get");
	cmdhash ("gets");
	cmdhash ("incr");
	cmdhash ("decr");
	cmdhash ("delete");
	cmdhash ("set");
	cmdhash ("replace");
	cmdhash ("add");
	cmdhash ("stats");
	cmdhash ("quit");
	return 0;
}
