/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "memcache.h"
#include <signal.h>
#include <sys/socket.h>

McSdb *ms = NULL;
int protocol_handle (McSdbClient *c, char *buf);

static void sigint() {
	signal (SIGINT, SIG_IGN);
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

static McSdbClient *mcsdb_client_new (int fd) {
	McSdbClient *c = R_NEW (McSdbClient);
	memset (c, 0, sizeof (McSdbClient));
	c->fd = fd;
	return c;
}

static int mcsdb_client_state(McSdbClient *c) {
	char *p;
	int r;
/*
	if (c->next>0) {
		memcpy (c->buf, c->buf+c->next, c->idx-c->next);
		printf ("next walk (next=%d) len=%d\n", c->next, c->idx-c->next);
		c->idx = c->next;
		c->next = 0;
	}
*/
	switch (c->mode) {
	case 0: // read until newline
		if (c->len+c->idx >= MEMCACHE_MAX_BUFFER) {
			*c->buf = 0;
			c->idx = 1; // invalid read, so just chop it
		}
		r = read (c->fd, c->buf+c->idx, MEMCACHE_MAX_BUFFER-c->idx);
printf ("READ %d %d\n", r, c->idx);
//		if (r>0 || c->idx>0) {
		if (1) { //c->idx>0) {
if (r>0) {
			ms->bread += r;
			c->idx += r;
}
			if ((p = strchr (c->buf, '\n'))) {
				char *rest = p+1;
				*p--=0;
				if (p>c->buf && *p=='\r')
					*p = 0;
printf ("PAD %d = (%s)\n" , r, c->buf);
printf ("REST %d = (%s)\n" , (int)(rest-c->buf), rest);
				c->next = (c->idx-(int)(rest-c->buf));
printf ("REST IS (%s)\n", c->buf+c->next);
				return 1;
			}
			return 0;
		}
		break;
	case 1: // read N bytes
printf ("GO READ N BYTES %d\n", c->len);
		if (c->len+c->idx >= MEMCACHE_MAX_BUFFER) {
			*c->buf = 0;
			c->idx = 1; // invalid read, so just chop it
		}
printf ("rest %d\n", c->next);
		r = read (c->fd, c->buf+c->idx, c->len-c->idx);
//c->mode = 0; // fuck yeah
		//if (r<1) return 0;
		if (r!=-1)
			c->idx += r;
		c->buf[c->idx] = 0;
printf ("BODY IS (%s)\n",c->buf);
		if (c->idx >= c->len) {
			return 1;
		}
		break;
	case 2: // write
		write (c->fd, c->buf+c->idx, c->len-c->idx);
		break;
	}
	return 0;
}

static void mcsdb_client_free (McSdbClient *c) {
	free (c);
}

static void fds_server (int fd) {
	ms->fds[0].fd = fd;
	ms->fds[0].events = POLLIN;
	ms->nfds = 1;
}

static int fds_add (int fd) {
	int n = ms->nfds;
	if (n>MEMCACHE_MAX_CLIENTS)
		return 0;
	ms->fds[n].fd = fd;
	ms->fds[n].events = POLLIN | POLLHUP | POLLERR;
	ms->fds[n].revents = 0;
	ms->msc[n] = mcsdb_client_new (fd);
	ms->nfds++;
	ms->tfds++;
	return 1;
}

static int fds_del (int fd) {
	int i;
	for (i=0; i<ms->nfds; i++) {
		if (ms->fds[i].fd == fd) {
			mcsdb_client_free (ms->msc[i]);
			for (++i; i<ms->nfds; i++) {
				ms->fds[i-1] = ms->fds[i];
				ms->msc[i-1] = ms->msc[i];
			}
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
					if (!fds_add (cfd)) {
						printf ("cannot accept more clients\n");
						net_close (cfd);
					} else printf ("new client %d\n", cfd);
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
			rework:
				// XXX: write op is not handled here
				if (mcsdb_client_state (ms->msc[i])) {
					strchop (buf, r);
					ms->msc[i]->idx = 0;
					if (protocol_handle (ms->msc[i], ms->msc[i]->buf)==-1) {
						fds_del (ms->fds[i].fd);
						goto respawn;
					}
				} else {
					printf ("no newline wtf\n");
					if (ms->msc[i]->next) {
					//	ms->msc[i]->idx = ms->msc[i]->next;
					} else {
						net_printf (ms->fds[i].fd, "ERROR\n");
						ms->msc[i]->idx = 0;
					}
				}
				net_flush (ms->fds[i].fd);
				if (ms->msc[i]->next==0) {
					ms->fds[i].revents = 0;
				} else {
McSdbClient *c = ms->msc[i];
					printf ("MUST READ AGAIN HOHO --> %d\n", c->next);
printf ("---> (%s)\n", c->buf+c->next+2);
int len;
if (c->idx>=c->next)
	len = c->idx-c->next;
else {
	fprintf (stderr, "GTFO %d %d\n", c->idx, c->next+1);
// XXX: THIS CONDITION IS WRONG :(
						fds_del (ms->fds[i].fd);
						goto respawn;
//goto rework;
continue;
}
memcpy (c->buf, c->buf+c->next, len);
printf ("NEWBUF IS %s\n", c->buf);
c->idx = c->next;
c->next = 0;
					goto rework;
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
	ret = net_loop (port);
	mcsdb_free (ms);
	return ret;
}
