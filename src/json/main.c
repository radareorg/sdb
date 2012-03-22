/* Copyleft 2012 - sdb (aka SimpleDB) - pancake<nopcode.org> */

#include <stdio.h>
#include <stdlib.h>
#include "rangstr.h"
#include "json.h"

int main(int argc, char **argv) {
	Rangstr rs;
	char buf[4096];
	int n = fread (buf, 1, sizeof (buf), stdin);
	buf[n] = 0;
	char *path = argv[1];

	printf (">>>> %s <<<<\n", sdb_json_unindent (buf));
	printf (">>>> %s <<<<\n", sdb_json_indent (buf));
// set value //
	path = "glossary.title";
	char *s = sdb_json_set (buf, path, "patata");
	if (s) {
		printf ("%s\n", s);
		free (s);
	} else printf ("set failed\n");
//printf ("%s\n", str); return 0;

//	s = "[1,3,4]";
#if 0
	char *a = "{}";
	a = json_seti (a, "id", 123);
	a = json_seti (a, "user.name", "blah");
	printf ("id = %d\n", json_geti ("{'id':123}", "id"));
#endif
	//json_walk (buf);

	path = argv[1];
	rs = json_get (buf, path);
	///	rs = rangstr_new (path);
	//	rs = json_find (str, &rs);
	rangstr_print (&rs);
	printf ("\n");
	return 0;
}
