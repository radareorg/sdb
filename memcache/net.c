/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include "types.h"
#include <netinet/in.h>
#include <fcntl.h>

int net_listen (int port) {
	int fd;
        struct sockaddr_in sa;
        struct linger linger = { 0 };

        if ((fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
                return R_FALSE;
        linger.l_onoff = 1;
        linger.l_linger = 1;
        setsockopt (fd, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof (linger));
        memset (&sa, 0, sizeof (sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        sa.sin_port = htons (port);

        if (bind (fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
                close (fd);
                return R_FALSE;
        }
        fcntl (fd, F_SETFL, O_NONBLOCK, 0); // //!block);
        signal (SIGPIPE, SIG_IGN);
        if (listen (fd, 1) < 0) {
                close (fd);
                return R_FALSE;
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
