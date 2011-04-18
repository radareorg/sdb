/* Public domain -- pancake @ 2011 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "sdb.h"

static int save = 0;
static sdb *s = NULL;

static void sigint(int sig) {
	if (!s) return;
	if (save) sdb_sync (s);
	sdb_free (s);
}

static void sdb_dump (sdb* s) {
	char k[SDB_KEYSIZE];
	char v[SDB_VALUESIZE];
	sdb_dump_begin (s);
	while (sdb_dump_next (s, k, v))
		printf ("%s=%s\n", k, v);
}

static void runline (sdb *s, const char *cmd) {
	ut64 n;
	char *p, *eq = strchr (cmd, '=');
	if (eq) {
		save = 1;
		*eq = 0;
		sdb_set (s, cmd, eq+1);
	} else {
		switch (*cmd) {
		case '+': // inc
			n = sdb_inc (s, cmd+1);
			save = 1;
			printf ("%lld\n", n);
			break;
		case '-': // dec
			n = sdb_dec (s, cmd+1);
			save = 1;
			printf ("%lld\n", n);
			break;
		default:
			p = sdb_get (s, cmd);
			if (p) {
				printf ("%s\n", p);
				free (p);
			}
		}
	}
}

int main(int argc, char **argv) {
	char line[1024];
	int i;
	if (argc<2) {
		printf ("usage: sdb [db] [-=]|[key[=value] ..]\n");
		return 1;
	}
	if (argc == 2) {
		s = sdb_new (argv[1], 0);
		sdb_dump (s);
		sdb_free (s);
		return 0;
	}
	signal (SIGINT, sigint);
	signal (SIGHUP, sigint);

	if (!strcmp (argv[2], "=")) {
		struct cdb_make c;
		const char *f = argv[1];
		char *eq, *ftmp = malloc (strlen (f)+5);
		sprintf (ftmp, "%s.tmp", f);
		int fd = open (ftmp, O_RDWR|O_CREAT|O_TRUNC, 0644);
		if (fd == -1) {
			printf ("cannot create %s\n", ftmp);
			exit (1);
		}
		cdb_make_start (&c, fd);
		for (;;) {
			fgets (line, sizeof line, stdin);
			if (feof (stdin))
				break;
			line[strlen (line)-1] = 0;
			eq = strchr (line, '=');
			if (eq) {
				*eq = 0;
				sdb_add (&c, line, eq+1);
			}
		}
		cdb_make_finish (&c);
		fsync (fd);
		close (fd);
		rename (ftmp, f);
		free (ftmp);
		//exit (0);
	} else
	if (!strcmp (argv[2], "-")) {
		s = sdb_new (argv[1], 0);
		// read from stdin to write db?
		for (;;) {
			fgets (line, sizeof line, stdin);
			if (feof (stdin))
				break;
			line[strlen (line)-1] = 0;
			runline (s, line);
		}
	} else {
		s = sdb_new (argv[1], 0);
		for (i=2; i<argc; i++)
			runline (s, argv[i]);
	}
	sigint (0);
	return 0;
}
