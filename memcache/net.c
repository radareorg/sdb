/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "types.h"
#include "memcache.h"
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>

static char netbuf[1024];
static int netbuflen = 0;

int net_flush(int fd) {
	int n;
	if (netbuflen<1) return;
	if (fd==-1) fd = 1; //stdout
	n = write (fd, netbuf, netbuflen);
	if (n>0) ms->bwrite += n;
	netbuflen = 0;
}

int net_printf (int fd, char *fmt, ...) {
	int n;
	char buf[1024];
	va_list ap;
	va_start (ap, fmt);
	n = vsnprintf (buf, sizeof (buf)-1, fmt, ap);
	if (netbuflen+n>sizeof (netbuf))
		net_flush (fd);
	strcpy (netbuf+netbuflen, buf);
	va_end (ap);
	netbuflen += n;
	return n;
}

int net_listen (int port) {
	int fd;
        struct sockaddr_in sa;
        struct linger linger = { 0 };

        if ((fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
                return -1;
        linger.l_onoff = 1;
        linger.l_linger = 1;
        setsockopt (fd, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof (linger));
        memset (&sa, 0, sizeof (sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl (INADDR_ANY);
        sa.sin_port = htons (port);

        if (bind (fd, (struct sockaddr *)&sa, sizeof (sa)) != 0) {
                close (fd);
                return -1;
        }
        fcntl (fd, F_SETFL, O_NONBLOCK, 0); // //!block);
        signal (SIGPIPE, SIG_IGN);
        if (listen (fd, 1) < 0) {
                close (fd);
                return -1;
        }
	return fd;
}

int net_close (int s) {
	shutdown (s, SHUT_RDWR);
	return close (s);
}
