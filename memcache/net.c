/* mcsdb - LGPLv3 - Copyright 2011-2014 - pancake */

#include "types.h"
#include "mcsdb.h"
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdarg.h>

static char netbuf[MCSDB_MAX_BUFFER];
static size_t netbuflen = 0;


char *net_readnl(int fd) {
	int i = -1;
	char buf[MCSDB_MAX_BUFFER];
	do {
		i++;
		if (i==sizeof (buf))
			return NULL;
		if (read (fd, buf+i, 1) != 1)
			return NULL;
	} while (buf[i]!='\n');
	buf[i-1] = 0;// chop \r
	return strdup (buf);
}

//NetSocket *net_accept(NetSocket *s) { }

int net_flush(int fd) {
	int n;
	if (netbuflen<1) return 0;
	if (fd==-1) fd = 1; //stdout
	n = write (fd, netbuf, netbuflen);
	netbuflen = 0;
	return n;
}

int net_printf (int fd, const char *fmt, ...) {
	int n;
	char buf[MCSDB_MAX_BUFFER];
	va_list ap;
	va_start (ap, fmt);
	n = vsnprintf (buf, sizeof (buf)-1, fmt, ap);
	va_end (ap);
	if (n < 0)
		return 0;
	if (netbuflen+n>sizeof (netbuf))
		net_flush (fd);
	strcpy (netbuf+netbuflen, buf);
	netbuflen += n;
	return n;
}

void net_sockopt (int fd) {
#if __linux__
	struct linger ling = {0, 0};
#endif
	int flags = 1;
        (void)setsockopt (fd, SOL_SOCKET, SO_REUSEADDR,
		(void *)&flags, sizeof (flags));
#if __linux__
	(void)setsockopt (fd, SOL_SOCKET, SO_LINGER,
		(void *)&ling, sizeof (ling));
#endif
}

int net_listen (int port) {
	int fd;
        struct sockaddr_in sa;

        if ((fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
                return -1;
	net_sockopt (fd);
        memset (&sa, 0, sizeof (sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl (INADDR_ANY);
        sa.sin_port = htons (port);

        if (bind (fd, (struct sockaddr *)&sa, sizeof (sa)) != 0) {
                close (fd);
                return -1;
        }
        (void)fcntl (fd, F_SETFL, O_NONBLOCK, 0);
        signal (SIGPIPE, SIG_IGN);
        if (listen (fd, 1) != -1)
		return fd;
	close (fd);
	return -1;
}

int net_close (int s) {
	shutdown (s, SHUT_RDWR);
	return close (s);
}

int net_connect(const char *host, const char *port) {
        struct addrinfo hints, *res, *rp;
        int s = -1, gai;
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_INET; //UNSPEC;            /* Allow IPv4 or IPv6 */
	hints.ai_protocol = IPPROTO_TCP;
	gai = getaddrinfo (host, port, &hints, &res);
	if (gai != 0) {
		printf ("Error in getaddrinfo: %s\n", gai_strerror (gai));
		return -1;
	}
	if (res == NULL) {
		printf ("Could not connect\n");
		return -1;
	}
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		s = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (s == -1)
			continue;
		if (connect (s, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		close (s);
		s = -1;
	}
        //fcntl (s, F_SETFL, O_NONBLOCK, 0); // //!block);
	freeaddrinfo (res);
	return s;
}
