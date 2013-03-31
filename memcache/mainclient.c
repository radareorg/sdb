/* mcsdb - LGPLv3 - Copyright 2011-2013 - pancake */

#include "mcsdb.h"

int main(int argc, char **argv) {
	McSdbClient *ms;
	char *p, buf[MCSDB_MAX_BUFFER];
	const char *host = argc>1? argv[1]: "localhost";
	const char *port = argc>2? argv[2]: "11211";
	if (argc>1 && !strcmp (argv[1], "-h")) {
		printf ("Usage: ./client [host] [port]\n");
		return 1;
	}
	ms = mcsdb_client_new (host, port);
	if (ms == NULL) {
		fprintf (stderr, "Cannot connect to %s %s\n", host, port);
		return 1;
	}
	for (;;) {
		if (fgets (buf, sizeof (buf), stdin) == NULL || feof (stdin))
			break;
		buf[strlen (buf)-1] = 0;
		if (buf[0]=='+') {
			mcsdb_client_incr (ms, buf+1, 1);
		} else
		if (buf[0]=='-') {
			mcsdb_client_decr (ms, buf+1, 1);
		} else {
			p = strchr (buf, '=');
			if (p) {
				*p++ = 0;
				if (*p) mcsdb_client_set (ms, buf, p, 0);
				else mcsdb_client_remove (ms, buf, 0);
			} else {
				char *v = mcsdb_client_get (ms, buf, NULL);
				printf ("%s\n", v? v: "");
			}
		}
	}
	mcsdb_client_free (ms);
	return 0;
}
