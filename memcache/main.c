#include "memcache.h"

static MemcacheSdb *ms = NULL;

static int whileread (int fd, char *b, int len) {
	int all = 0;
	int ret;
	for (;len>0;) {
		ret = fread (b, 1, len, stdin);
		if (ret==-1) return -1;
		if (ret>0) {
			b += ret;
			all += ret;
			len -= ret;
		}
	}
	return all;
}

static void sigint() {
	fprintf (stderr, "SIGINT handled.\n");
	memcache_free (ms);
	exit (0);
}

static void setup_signals() {
	signal (SIGINT, sigint);
}

static void main_version() {
	printf ("mcsdbd v"MEMCACHE_VERSION"\n");
}

static void main_help(const char *arg) {
	printf ("mcsdbd [-hv] [-p port] [sdbfile]\n");
}

static void handle_get(MemcacheSdb *ms, char *key, int smode) {
	ut64 exptime = 0LL;
	int n = 0;
	char *k, *K = key;
	for (;;) {
		k = strchr (K, ' ');
		if (k) *k=0;
		// TODO: split key with spaces
		char *s = memcache_get (ms, K, &exptime);
		if (s) {
			if (smode)
				printf ("VALUE %s %llu 0 %d\r\n", K, exptime, (int)strlen (s));
			else printf ("VALUE %s %llu %d\r\n", K, exptime, (int)strlen (s));
			printf ("%s\r\nEND\r\n", s);
			n++;
			free (s);
		}
		if (k) K = k+1;
		else break;
	}
	if (!n) printf ("END\r\n");
}

static int handle (char *buf) {
	char *p, *cmd = buf, *key, tmp[100], tmp2[100];
	int flags = 0, bytes = 0;
	ut64 exptime = 0LL;

	if (!*buf) {
		printf ("ERROR\r\n");
		return 0;
	}
	p = strchr (buf, ' ');
	if (p) {
		*p = 0;
		key = p + 1;
	}
	if (!strcmp (cmd, "quit")) {
		return -1;
	} else
	if (!strncmp (buf, "gets ", 5)) {
		handle_get (ms, key, 1);
	} else
	if (	!strcmp (cmd, "incr") ||
		!strcmp (cmd, "decr")
			) {
		char *ret;
		ut64 n = 0;
		p = strchr (key, ' ');
		if (!p) {
			return 0;
		}
		*p++ = 0;
		sscanf (p, "%llu", &n);
		if (*cmd=='i') ret = memcache_incr (ms, key, n);
		else ret = memcache_decr (ms, key, n);
		if (ret) printf ("%s\r\n", ret);
		else printf ("SERVER_ERROR numeric overflow\r\n");
		free (ret);
	} else
	if (!strcmp (cmd, "get")) {
		handle_get (ms, key, 0);
	} else
	if (!strcmp (cmd, "delete")) {
		p = strchr (key, ' ');
		if (!p) {
			return 0;
		}
		*p = 0;
		exptime = 0LL;
		sscanf (p+1, "%llu", &exptime);
		if (memcache_delete (ms, key, exptime))
			printf ("DELETED\r\n");
		else printf ("NOT_FOUND\r\n");
	} else
	if (	!strcmp (cmd, "add") ||
		!strcmp (cmd, "set") ||
		!strcmp (cmd, "append") ||
		!strcmp (cmd, "prepend") ||
		!strcmp (cmd, "replace")
			) {
		int stored = 1;
		char *b;
		p = strchr (key, ' ');
		if (!p) {
			return 0;
		}
		*p = 0;
		sscanf (p+1, "%d %llu %d", &flags, &exptime, &bytes);
		if (bytes<1) {
			printf ("CLIENT_ERROR invalid length\r\n");
			printf ("ERROR\r\n");
			return 0;
		}
		bytes++; // '\n'
		b = malloc (bytes+1);
		if (b) {
			int ret = whileread (0, b, bytes); // XXX
			if (ret != bytes) {
				printf ("CLIENT_ERROR bad data chunk\r\n");
				printf ("ERROR\r\n");
				return 0;
			}
		} else {
			printf ("CLIENT_ERROR invalid\r\n");
			printf ("ERROR\r\n");
			return 0;
		}
		if (feof (stdin))
			return -1;
		b[--bytes] = 0;
		switch (*cmd) {
		case 's': memcache_set (ms, key, exptime, b); break;
		case 'a': if (cmd[1]=='p') memcache_append (ms, key, exptime, b);
			else stored = memcache_add (ms, key, exptime, b); break;
		case 'p': memcache_prepend (ms, key, exptime, b); break;
		case 'r': stored = memcache_replace (ms, key, exptime, b); break;
		}
		if (stored) printf ("STORED\r\n");
		else printf ("NOT_STORED\r\n");
		return 1;
	} else printf ("ERROR\r\n");
	return 0;
}

static void main_loop() {
	char buf[100];
	for (;;) {
		fgets (buf, sizeof (buf)-1, stdin);
		if (feof (stdin)) break;
		buf[ strlen (buf)-1 ] = 0;
		if (handle (buf)==-1) break;
		fflush (stdout);
	}
}

int main(int argc, char **argv) {
	const char *file = MEMCACHE_FILE;
	int port = MEMCACHE_PORT;
	char c;

	while ((c = getopt (argc, argv, "hvp:")) != -1) {
		switch (c) {
		case 'h': main_help (argv[0]); return 0;
		case 'v': main_version (); return 0;
		case 'p': port = atoi (optarg); break;
		}
	}
	if (port<1) {
		fprintf (stderr, "Invalid port %d\n", port);
		return 1;
	}
	if (optind < argc)
		file = argv[optind];
	setup_signals ();
	ms = memcache_sdb_new (file);
	main_loop ();
	memcache_free (ms);
	return 0;
}
