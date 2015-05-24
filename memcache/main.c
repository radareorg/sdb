/* mcsdb - LGPLv3 - Copyright 2011-2015 - pancake */

#include <signal.h>
#include "mcsdb.h"
#include "../src/types.h"
#include <fcntl.h>
#if __SDB_WINDOWS__
#include <windows.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

static McSdb *ms = NULL;

static McSdbClient *mcsdb_client_new_fd(int fd) {
	McSdbClient *c = R_NEW (McSdbClient);
	if (!c) return NULL;
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

static void sigint(int sig UNUSED) {
	signal (SIGINT, SIG_IGN);
	eprintf ("gets %lld\n", ms->gets);
	eprintf ("sets %lld\n", ms->sets);
	eprintf ("hits %lld\n", ms->hits);
	eprintf ("miss %lld\n", ms->misses);
	eprintf ("read %lld\n", ms->bread);
	eprintf ("writ %lld\n", ms->bwrite);
	eprintf ("nfds %d\n", ms->nfds);
	eprintf ("SIGINT handled.\n");
	mcsdb_free (ms);
	exit (0);
}

static void setup_signals(void) {
	signal (SIGINT, sigint);
}

static void main_version(void) {
	printf ("mcsdbd v"MCSDB_VERSION"\n");
}

static void main_help(const char *arg UNUSED) {
	printf ("mcsdbd [-hv] [-p port] [sdbfile]\n");
}

static int mcsdb_client_accept(int fd) {
	int cfd = accept (fd, NULL, NULL);
	if (cfd != -1) {
		if (fds_add (cfd)) {
			ms->fds[0].revents = 0;
			return 1;
		}
		eprintf ("cannot accept more clients\n");
		net_close (cfd);
	}
	return 0;
}

// XXX: write op is not handled here
static int mcsdb_client_state(McSdbClient *c) {
	char *p;
	int r, rlen;

	if (c->next>0) {
		//int clen = c->idx-c->next;
		int clen = strlen (c->buf+c->next)+1;
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
		rlen = MCSDB_MAX_BUFFER-1 - c->idx;
		r = c->next? 0: read (c->fd, c->buf+c->idx, rlen);
		if (r<0) {
			eprintf ("ignored error\n");
			return 1;
		}
		c->buf[c->idx+r] = 0;
		if ((p = strchr (c->buf+c->idx, '\n'))) {
			*p--=0;
			if (p>c->buf && *p=='\r')
				*p = 0;
			const int restlen = (int)(size_t)(p-c->buf);
			c->next = restlen+2;
			c->idx += r;
			return 1;
		}
		ms->bread += r;
		c->idx += r;
		return 1;
	case 1: // read N bytes
		if (c->idx>c->len)
			c->idx = 0;
		r = 0;
		rlen = c->len-c->idx;
		if (rlen>0 && c->len>0) {
			r = read (c->fd, c->buf+c->idx, rlen);
			if (r==-1) {
			// 	perror ("read");
			}
			if (r != rlen) {
				c->buf[sizeof (c->buf)-1] = 0;
				c->idx = 0;
				// shift internal buffer for bulk writes
				if (0 == r) {
					strncpy (c->buf, c->buf+c->len,
						sizeof (c->buf)-1);
					c->len = strlen (c->buf);
					return 1;
				}
				return 0;
			}// else eprintf ("Invalid %d read wtf\n", r);
		} 
		c->idx += r;
		c->buf[c->idx+1] = 0;
		if (c->idx == c->len) {
		//	printf ("END OF MODE 1 ***/*/*///*/* ((%s))\n", c->buf);
			return 1;
		}
		break;
	case 2:
		r = write (c->fd, c->buf+c->idx, c->len-c->idx);
		if (r!=(c->len-c->idx)) return 1;
		break;
	}
	return 0;
}

static void fds_server (int n, int fd) {
	ms->fds[n].fd = fd;
	ms->fds[n].events = POLLIN;
	ms->nfds = n+1;
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

static int udp_listen (int port) {
	struct sockaddr_in si_me;
	int s;

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		return -1;

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (s, (struct sockaddr*)&si_me, sizeof(si_me))==-1) {
		close (s);
		return -1;
	}
//fcntl (s, F_SETFL, O_NONBLOCK, 0);
	return s;
}

static int udp_parse(McSdbClient *c UNUSED, int fd) {
#pragma pack(2)
#define ut16 unsigned short
struct udphdr_t {
	ut16 rid; // request id
	ut16 seq; // sequence number
	ut16 ndg; // number of datagrams
	ut16 res; // reserved for future use
};
#pragma pack()
struct udphdr_t h;
	char buf[32768];
	int ret = read (fd, buf, sizeof (buf)-1);
	if (ret<1) return 0;
	buf[ret-1] = 0;
	memcpy (&h, buf, 8);
#if 0
0-1 Request ID
2-3 Sequence number
4-5 Total number of datagrams in this message
6-7 Reserved for future use; must be 0
#endif
	//printf ("UDP (%s)\n", buf+8); // TODO
	if (!memcmp (buf+8, "set ", 4)) {
		int a, b, l = 0;
		char *n, *p = strchr (buf+12, ' ');
		if (p) {
			n = strchr (p, '\n');
			if (n != p) {
				*p = 0;
				if (n) *n++ = 0;
				sscanf (p+1, "%d %d %d", &a, &b, &l);
#if 1
				if (n) {
					int nlen = sizeof (buf)-1;
					int left = nlen - (size_t)(n-p);
					if (l>=0 && l<left)
						n[l] = 0;
					else buf[sizeof (buf)-1] = 0;
				}
#else
				if (n) n[l] = 0;
#endif
				//	printf ("KEY %s\n", buf+12);
				//	printf ("SET %s\n", n);
				mcsdb_set (ms, buf+12, n, 0, 0);
			}
		}
	}
#if 0
set Xi5HaaaJ1nLnZGOEbkou9D7iuSc2fL4239JTclgdY2O5AgKeG0OP5X0zrStMppOkyjDQSu3soJyyrAc8A0PFNP67lrTaguAWTOxq 0 0 400 noreply
A0iqCgg3i9Bs1TzHmxOYotySn9z9PwrRoZ9s6pwoyzZsKY97MZXAQTX6eM7vAyMyXXjTDXI3Mjvj74iTdDYwi3uDP18bzNPMm8srf2vulqdsnJMGYu46pyJEzKhyb6LDF55m7sgsa9cLSqWscazPZIXYWCNxJcBQh84g0lOam0o7rIzJkylH98iXL5TgZgyEo2ugfIEQAuZt4ODpNq6OygvJlGrIwpOcsA8NSFpX9EOEfT3uJ1IAi5LJEDWAuufexn2H2jo4y5ITaDNu6ZwgWH0kWMnH8Qv55xN0ZB4XGEsjJFTHgPyAiqv5CiMK0HIx7hxgkt3tzvcA2xSiMIsw8n1CLFODPiBYN040u7lluLowI8eYQMSQAMeNd2d2loQ0oWsaTdCNq0B88pYQ
#endif
	return 1;
}

static int net_loop(int port) {
	int i0, udpfd, ret, i, r, fd = net_listen (port);
	if (fd==-1) {
		printf ("cannot listen on tcp %d\n", port);
		return 1;
	}
	udpfd = udp_listen (port);
	printf ("listening on %d tcp/udp\n", port);
	fds_server (0, fd);
	if (udpfd != -1) {
		fds_server (1, udpfd);
		ms->fds[1].events = POLLIN;
		i0 = 2;
	} else i0 = 1;

	for (;;) {
		for (i=0;i<ms->nfds; i++)
			ms->fds[i].revents = 0;
		r = poll (ms->fds, ms->nfds, -1);
		if (r<1)
			continue;
		if (ms->fds[0].revents) {
			mcsdb_client_accept (fd);
			//continue;
		}
		if (udpfd != -1)
		if (ms->fds[1].revents) { // == POLLIN) {
			// XXX HIHGLY INEFFICIENT
			McSdbClient *c = mcsdb_client_new_fd (udpfd);
			udp_parse (c, udpfd);
			free (c);
			//continue;
		}
	respawn:
		for (i=i0; i<ms->nfds; i++) {
			McSdbClient *c;
			/* skip no events for this fd */
			if (!ms->fds[i].revents)
				continue;
			c = ms->msc[i];
			do {
				ret = mcsdb_client_state (c);
				int ph = protocol_handle (ms, c, c->buf);
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
					ms->fds[i].revents = 0;
					goto respawn;
				}
			} while (ret);
			r = net_flush (ms->fds[i].fd);
			if (r>0)
				ms->bwrite += r;
			/* client closed the connection */
			if (ms->fds[i].revents & POLLHUP) {
				fds_del (c);
				ms->fds[i].revents = 0;
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
		eprintf ("Invalid port %d\n", port);
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
