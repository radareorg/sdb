/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "memcache.h"
#include <signal.h>

MemcacheSdb *ms = NULL;
int protocol_handle (int fd, char *buf);

static void sigint() {
	fprintf (stderr, "SIGINT handled.\n");
	mcsdb_free (ms);
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
		if (protocol_handle (-1, buf)==-1) break;
		fflush (stdout);
	}
}

static void fds_server (int fd) {
	ms->fds[0].fd = fd;
	ms->fds[0].events = POLLIN;
	ms->nfds = 1;
}

static int fds_add (int fd) {
	int n = ms->nfds;
	if (n+1>=MEMCACHE_MAX_CLIENTS)
		return 0;
	ms->fds[n].fd = fd;
	ms->fds[n].events = POLLIN | POLLHUP | POLLERR;
	ms->fds[n].revents = 0;
	ms->nfds++;
	ms->tfds++;
	return 1;
}

static int fds_del (int fd) {
	int i;
	for (i=0; i<ms->nfds; i++) {
		if (ms->fds[i].fd == fd) {
			for (++i; i<ms->nfds; i++)
				ms->fds[i-1] = ms->fds[i];
			ms->nfds--;
			close (fd);
			return 0;
		}
	}
	return 1;
}

static void strchop(char *buf, int len) {
	while (buf[len-1] == '\r' || buf[len-1] == '\n')
		len--;
	buf[len] = 0; // XXX: fix chop
}

static int net_loop(int port) {
	char buf[1024];
	int i, r, fd = net_listen (port);
	if (fd==-1) {
		printf ("cannot listen on %d\n", port);
		return 1;
	}
	printf ("listening on %d\n", port);
	fds_server (fd);
	for (;;) {
		r = poll (ms->fds, ms->nfds, -1);
		if (r>0) {
			if (ms->fds[0].revents) {
				int cfd = accept (fd, NULL, NULL);
				if (cfd != -1) {
					fds_add (cfd);
					printf ("new client %d\n", cfd);
				} else printf ("ACCEPT FAIL\n");
				ms->fds[0].revents = 0;
				continue;
			}
		respawn:
			for (i=1; i<ms->nfds; i++) {
				if (!ms->fds[i].revents)
					continue;
				if (ms->fds[i].revents & POLLHUP) {
					fds_del (ms->fds[i].fd);
					goto respawn;
				}
				ms->fds[i].revents = 0;
				r = read (ms->fds[i].fd, buf, sizeof (buf)-1);
				if (r>2) {
					strchop (buf, r);
					if (protocol_handle (ms->fds[i].fd, buf)==-1) {
						fds_del (ms->fds[i].fd);
						goto respawn;
					}
				}
			}
		}
	}
	net_close (fd);
	return 0;
}

int main(int argc, char **argv) {
	const char *file = MEMCACHE_FILE;
	int port = MEMCACHE_PORT;
	char c, ret = 0;

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
	ms = mcsdb_new (file);
	//main_loop ();
	ret = net_loop (port);
	mcsdb_free (ms);
	return ret;
}
