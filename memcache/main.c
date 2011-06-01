/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "memcache.h"
#include <signal.h>

MemcacheSdb *ms = NULL;
int protocol_handle (char *buf);

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

static void main_loop() {
	char buf[256];
	for (;;) {
		fgets (buf, sizeof (buf)-1, stdin);
		if (feof (stdin)) break;
		buf[ strlen (buf)-1 ] = 0;
		if (protocol_handle (buf)==-1) break;
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
