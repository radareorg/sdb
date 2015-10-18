/* mcsdb - LGPLv3 - Copyright 2011-2015 - pancake */

#include "sdb.h"
#include <ctype.h>

static void cmdhash(const char *s) {
	unsigned int i;
	char su[100];
	for (i=0; s[i]; i++)
		su[i] = toupper ((unsigned char)s[i]);
	su[i] = 0;
	printf ("#define MCSDB_CMD_%s 0x%x\n", su, sdb_hash (s));
}

int main(void) {
	cmdhash ("version");
	cmdhash ("verbosity");
	cmdhash ("flush_all");
	cmdhash ("append");
	cmdhash ("prepend");
	cmdhash ("get");
	cmdhash ("gets");
	cmdhash ("incr");
	cmdhash ("decr");
	cmdhash ("delete");
	cmdhash ("set");
	cmdhash ("replace");
	cmdhash ("cas");
	cmdhash ("add");
	cmdhash ("stats");
	cmdhash ("quit");
	cmdhash ("touch");
	return 0;
}
