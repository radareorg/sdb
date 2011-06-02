/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "types.h"
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdarg.h>

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

int net_readn (int s, char *b, int l) {
	// TODO: while here
	return read (s, b, l);
}

int net_poll() {
	struct pollfd fds;
//      r = poll (fds, fds_count, -1);
}

int net_printf (int fd, char *fmt, ...) {
	char buf[1024];
	va_list ap;
	va_start (ap, fmt);
	if (fd != -1) {
		int n = vsnprintf (buf, sizeof (buf)-1, fmt, ap);
		write (fd, buf, n);
	} else return vprintf (fmt, ap);
	va_end (ap);
}
