/* Copyleft 2011-2013 - sdb - pancake */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "sdb.h"

int sdb_queryf (Sdb *s, const char *fmt, ...) {
        char string[4096];
        int ret;
        va_list ap;
        va_start (ap, fmt);
        vsnprintf (string, sizeof (string), fmt, ap);
        ret = sdb_query (s, string);
        va_end (ap);
        return ret;
}

// TODO: return char *
//int sdb_querys (Sdb *s, const char *cmd) { }

int sdb_query (Sdb *s, const char *cmd) {
	char *p, *eq, *ask = strchr (cmd, '?');
	const char *p2;
	int i, save = 0;
	ut64 n;

	switch (*cmd) {
	case '(': // inc
		p = strchr (cmd, ')');
		if (p) {
			*p = 0;
			eq = strchr (p+1, '=');
			if (cmd[1]=='?') {
				printf ("%d\n", sdb_alength (s, p+1));
			} else
			if (cmd[1]) {
				i = atoi (cmd+1);
				if (eq) {
					*eq = 0;
					if (eq[1]) {
						if (cmd[1]=='+') {
							sdb_ains (s, p+1, i, eq+1);
						} else {
							sdb_aset (s, p+1, i, eq+1);
						}
					} else {
						sdb_adel (s, p+1, i);
					}
				} else {
					char *val = sdb_aget (s, p+1, i);
					if (val) {
						printf ("%s\n", val);
						free (val);
					}
				}
			} else {
				if (eq) {
					char *q, *out = strdup (eq+1);
					*eq = 0;
					// TODO: define new printable separator character
					for (q=out;*q;q++) if (*q==',') *q = SDB_RS;
					sdb_set (s, p+1, out, 0);
					free (out);
				} else {
					sdb_alist (s, p+1);
				}
			}
		} else fprintf (stderr, "Missing ')'.\n");
		break;
	case '+': // inc
		n = ask? 
			sdb_json_inc (s, cmd+1, ask, 1, 0):
			sdb_inc (s, cmd+1, 1, 0);
		printf ("%"ULLFMT"d\n", n);
		save = 1;
		break;
	case '-': // dec
		n = ask? 
			sdb_json_dec (s, cmd+1, ask, 1, 0):
			sdb_dec (s, cmd+1, 1, 0);
		printf ("%"ULLFMT"d\n", n);
		save = 1;
		break;
	default:
		eq = strchr (cmd, '=');
		if (eq && ask>eq) ask = NULL;
		if (eq) {
			// 1 0 kvpath=value
			// 1 1 kvpath?jspath=value
			save = 1;
			*eq++ = 0;
			if (ask) {
				// sdbjsonset
				*ask++ = 0;
				sdb_json_set (s, cmd, ask, eq, 0);
			} else {
				// sdbset
				sdb_set (s, cmd, eq, 0);
			}
		} else {
			// 0 1 kvpath?jspath
			// 0 0 kvpath
			if (ask) {
				// sdbjsonget
				*ask++ = 0;
				if ((p = sdb_json_get (s, cmd, ask, 0))) {
					printf ("%s\n", p);
					free (p);
				}
			} else {
				// sdbget
				if ((p2 = sdb_getc (s, cmd, 0)))
					printf ("%s\n", p2);
			}
		}
	}
	return save;
}
