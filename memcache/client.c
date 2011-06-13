#include "memcache.h"

static int fd;

McSdb *mcsdb_client_connect (const char *host, const char *port) {
	McSdb *ms;
	int fd = net_connect (host, port);
	if (fd == -1)
		return NULL;
	ms = mcsdb_new (NULL);	
	ms->fd = fd; 
	return ms;
}

char *mcsdb_client_incr(McSdb *ms, const char *key, ut64 val) {
	net_printf (ms->fd, "incr %s %lld\r\n", key, val);
	net_flush (ms->fd);
	return net_readnl (ms->fd);
}

char *mcsdb_client_decr(McSdb *ms, const char *key, ut64 val) {
	net_printf (ms->fd, "decr %s %lld\r\n", key, val);
	net_flush (ms->fd);
	return net_readnl (ms->fd);
}

void mcsdb_client_set(McSdb *ms, const char *key, const char *body, ut64 exptime) {
	net_printf (ms->fd, "set %s 0 0 %d\r\n", key, strlen (body));
	net_flush (ms->fd);
	net_printf (ms->fd, "%s\r\n", body);
	net_flush (ms->fd);
	free (net_readnl (ms->fd));
}

static int sendcmd (int fd, const char *cmd, const char *key, const char *body, ut64 exptime) {
	int ret;
	char *res;
	net_printf (fd, "add %s 0 0 %d\r\n", key, strlen (body));
	net_flush (fd);
	net_printf (fd, "%s\r\n", body);
	net_flush (fd);
	res = net_readnl (fd);
	ret = strstr (res, "NOT")? 0: 1;
	free (res);
	return ret;
}

int mcsdb_client_add(McSdb *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "add", key, body, exptime);
}

int mcsdb_client_append(McSdb *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "append", key, body, exptime);
}

int mcsdb_client_prepend(McSdb *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "prepend", key, body, exptime);
}

int mcsdb_client_replace(McSdb *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "replace", key, body, exptime);
}

char *mcsdb_client_get (McSdb *ms, const char *key, ut64 *exptime) {
	int sexp, slen = 0;
	char *buf, *p;
	net_printf (ms->fd, "get %s\r\n", key);
	net_flush (ms->fd);
	buf = net_readnl (ms->fd);
	if (!buf)
		return NULL;

	buf[strlen(buf)]=0;
	if (!memcmp (buf, "END", 3)) {
		free (buf);
		return NULL;
	}
	p = strchr (buf, ' ');
	if (p && (p = strchr (p+1, ' '))) {
		sscanf (p+1, "%d %d", &sexp, &slen);
	} else {
		free (buf);
		return NULL;
	}
	free (buf);
	if (exptime)
		*exptime = sexp;
	if (slen<1)
		return NULL;

	buf = malloc (slen+2);
	read (ms->fd, buf, slen+2);
	buf[slen] = 0;
	free (net_readnl (ms->fd)); // read END
	return buf;
}

int mcsdb_client_delete(McSdb *ms, const char *key, ut64 exptime) {
	int ret;
	char *res;
	net_printf (ms->fd, "delete %s %lld 0\r\n", key, exptime);
	net_flush (ms->fd);
	res = net_readnl (ms->fd);
	ret = strstr (res, "NOT")? 0: 1;
	free (res);
	return ret;
}

int main(int argc, char **argv) {
	McSdb *ms;
	char *p, buf[MCSDB_MAX_BUFFER];
	const char *host = argc>1? argv[1]: "localhost";
	const char *port = argc>2? argv[2]: "11211";
	if (argc>1 && !strcmp (argv[1], "-h")) {
		printf ("Usage: ./client [host] [port]\n");
		return 1;
	}
	ms = mcsdb_client_connect (host, port);
	if (ms == NULL) {
		fprintf (stderr, "Cannot connect to %s %s\n", host, port);
		return 1;
	}
	for (;;) {
		fgets (buf, sizeof (buf), stdin);
		if (feof (stdin))
			break;
		buf[strlen (buf)-1] = 0;
		if (buf[0]=='+') {
			mcsdb_client_incr (ms, buf+1, 1);
		} else
		if (buf[0]=='-') {
			mcsdb_client_decr (ms, buf+1, 1);
		} else {
			p = strchr (buf, '=');
			if (p) {
				*p++ = 0;
				if (*p)
					mcsdb_client_set (ms, buf, p, 0);
				else mcsdb_client_delete (ms, buf, 0);
			} else {
				char *v = mcsdb_client_get (ms, buf, NULL);
				printf ("%s\n", v? v: "");
			}
		}
	}
	net_close (fd);
	mcsdb_free (ms);
	return 0;
}
