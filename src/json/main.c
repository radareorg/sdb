#include <stdio.h>
#include "rangstr.h"
#include "json.h"

int main(int argc, char **argv) {
	char buf[4096];
	int n = fread (buf, 1, sizeof (buf), stdin);
	buf[n] = 0;

	char *path = "ping.board.sysid";
//printf ("%s\n", str); return 0;

	Rangstr rs;
//	s = "[1,3,4]";
#if 0
	char *a = "{}";
	a = json_seti (a, "id", 123);
	a = json_seti (a, "user.name", "blah");
	printf ("id = %d\n", json_geti ("{'id':123}", "id"));
#endif
	//json_walk (buf);

	path = "ping.network.dns[0]";
	path = argv[1];
	rs = json_get (buf, path);
	///	rs = rangstr_new (path);
	//	rs = json_find (str, &rs);
	rangstr_print (&rs);
	printf ("\n");
	return 0;
}
