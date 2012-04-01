/* Copyleft 2011 - mcsdb (aka memcache-SimpleDB) - pancake<nopcode.org> */
#include <signal.h>
#include <sys/socket.h>
#include "mcsdb.h"

McSdb *ms = NULL;
int protocol_handle(McSdbClient *c, char *buf);

static McSdbClient *mcsdb_client_new_fd(int fd) {
	McSdbClient *c = R_NEW (McSdbClient);
	memset (c, 0, sizeof (McSdbClient));
	c->fd = fd;
	return c;
}

static int fds_add(int fd) {
	int n = ms->nfds;
	if (n>MCSDB_MAX_CLIENTS)
		return 0;
	ms->fds[n].fd = fd;
	ms->fds[n].events = POLLIN | POLLHUP | POLLERR;
	ms->msc[n] = mcsdb_client_new_fd (fd);
	ms->nfds++;
	ms->tfds++;
	return 1;
}

static void sigint() {
	signal (SIGINT, SIG_IGN);
	fprintf (stderr, "gets %lld\n", ms->gets);
	fprintf (stderr, "sets %lld\n", ms->sets);
	fprintf (stderr, "hits %lld\n", ms->hits);
	fprintf (stderr, "miss %lld\n", ms->misses);
	fprintf (stderr, "read %lld\n", ms->bread);
	fprintf (stderr, "writ %lld\n", ms->bwrite);
	fprintf (stderr, "nfds %d\n", ms->nfds);
	fprintf (stderr, "SIGINT handled.\n");
	mcsdb_free (ms);
	exit (0);
}

static void setup_signals() {
	signal (SIGINT, sigint);
}

static void main_version() {
	printf ("mcsdbd v"MCSDB_VERSION"\n");
}

static void main_help(const char *arg) {
	printf ("mcsdbd [-hv] [-p port] [sdbfile]\n");
}

static int mcsdb_client_accept(int fd) {
	int cfd = accept (fd, NULL, NULL);
	if (cfd != -1) {
		if (fds_add (cfd)) {
			ms->fds[0].revents = 0;
			return 1;
		}
		fprintf (stderr, "cannot accept more clients\n");
		net_close (cfd);
	}
	return 0;
}

// XXX: write op is not handled here
static int mcsdb_client_state(McSdbClient *c) {
	char *p;
	int r, rlen;

	if (c->next>0) {
		int clen = c->idx-c->next;
		clen = strlen (c->buf+c->next)+1;
		memmove (c->buf, c->buf+c->next, clen);
		c->idx = clen-1;
		c->next = 0;
	}
	if (c->len+c->idx >= MCSDB_MAX_BUFFER) {
		*c->buf = 0;
		c->idx = 0; // invalid read, so just chop it
	}
	switch (c->mode) {
	case 0: // read until newline
		rlen = MCSDB_MAX_BUFFER - c->idx;
		if (c->next) {
			// never happens
			r = 0;
		} else {
			r = read (c->fd, c->buf+c->idx, rlen);
		}
		if (r<0) {
			printf ("ignored error\n");
			r = 0;
			return 1;
			//return 0;
		}
//		if (r>0) {
			c->buf[c->idx+r] = 0;
			//printf ("---- (%s)\n", c->buf+c->idx);
			if ((p = strchr (c->buf+c->idx, '\n'))) {
				//char *rest = p+1;
				*p--=0;
				if (p>c->buf && *p=='\r')
					*p = 0;
				int restlen = (int)(size_t)(p-c->buf); //(int)r-(rest-c->buf);
				c->next = restlen+2; //(c->idx-restlen);
				//printf ("MUST REREAD (%s) %d %d\n", c->buf, c->idx, c->next);
				//printf (" - --- next (%s) %d (len=%d)\n", c->buf+c->next, c->idx, c->idx);
				c->idx += r;
				return 1;
			}
			ms->bread += r;
			c->idx += r;
//		}
		return 1;
		// IGNORED return 0;
	case 1: // read N bytes
		if (c->idx>c->len)
			c->idx = 0;
		r = 0;
		rlen = c->len-c->idx;
		if (rlen>0 && c->len>0) {
			r = read (c->fd, c->buf+c->idx, rlen);
			if (r<1) {
				c->idx = 0;
				// shift internal buffer for bulk writes
				if (0 == r) {
					strcpy (c->buf, c->buf+c->len);
					c->len = strlen (c->buf);
					return 1;
				}
				return 0;
			}
		} 
		c->idx += r;
		c->buf[c->idx+1] = 0;
		if (c->idx == c->len) {
		//	printf ("END OF MODE 1 ***/*/*///*/* ((%s))\n", c->buf);
			return 1;
		}
		break;
	case 2: // write : not yet used
		write (c->fd, c->buf+c->idx, c->len-c->idx);
		break;
	}
	return 0;
}

static void fds_server (int fd) {
	ms->fds[0].fd = fd;
	ms->fds[0].events = POLLIN;
	ms->nfds = 1;
}

static int fds_del (McSdbClient *c) {
	int i, fd = c->fd;
	c->len = c->idx = 0;
	*c->buf = 0;
	for (i=0; i<ms->nfds; i++) {
		if (ms->fds[i].fd == fd) {
			mcsdb_client_free (c);
			for (++i; i<ms->nfds; i++) {
				ms->fds[i-1] = ms->fds[i];
				ms->msc[i-1] = ms->msc[i];
			}
			ms->nfds--;
			return 0;
		}
	}
	return 1;
}

static int net_loop(int port) {
	int ret, i, r, fd = net_listen (port);
	if (fd==-1) {
		printf ("cannot listen on %d\n", port);
		return 1;
	}
	printf ("listening on %d\n", port);
	fds_server (fd);
	for (;;) {
		r = poll (ms->fds, ms->nfds, -1);
		if (r<1)
			continue;
		if (ms->fds[0].revents) {
			mcsdb_client_accept (fd);
			continue;
		}
	respawn:
		for (i=1; i<ms->nfds; i++) {
			McSdbClient *c = ms->msc[i];
			/* skip no events for this fd */
			if (!ms->fds[i].revents)
				continue;
			do {
				ret = mcsdb_client_state (c);
//printf ("BOING next=%d idx=%d (%s)\n", c->next, c->idx, c->buf+c->idx);
//printf (".......................................... RET = %d\n", ret);
				int ph = protocol_handle (c, c->buf); //+c->next);
				switch (ph) {
				case 1:
					break;
				case 0:
					ret = 0;
					c->idx = 0;
					c->next = 0;
					break;
				case -1:
					fds_del (c);
					goto respawn;
				}
			} while (ret);
			r = net_flush (ms->fds[i].fd);
			if (r>0)
				ms->bwrite += r;
			/* client closed the connection */
			if (ms->fds[i].revents & POLLHUP) {
				fds_del (c);
				goto respawn;
			}
		}
	}
	net_close (fd);
	return 1;
}

int main(int argc, char **argv) {
	const char *file = MCSDB_FILE;
	int port = MCSDB_PORT;
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
