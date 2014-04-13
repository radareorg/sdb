/* mcsdb - LGPLv3 - Copyright 2011-2014 - pancake */

#include "mcsdb.h"

McSdbClient *mcsdb_client_new (const char *host, const char *port) {
	McSdbClient *ms;
	int fd = net_connect (host, port);
	if (fd == -1)
		return NULL;
	ms = R_NEW (McSdbClient);	
	memset (ms, 0, sizeof (McSdbClient));
	ms->fd = fd; 
	return ms;
}

void mcsdb_client_free (McSdbClient* ms) {
	net_close (ms->fd);
	free (ms);
}

char *mcsdb_client_incr (McSdbClient *ms, const char *key, ut64 val) {
	net_printf (ms->fd, "incr %s %lld\r\n", key, val);
	net_flush (ms->fd);
	return net_readnl (ms->fd);
}

char *mcsdb_client_decr (McSdbClient *ms, const char *key, ut64 val) {
	net_printf (ms->fd, "decr %s %lld\r\n", key, val);
	net_flush (ms->fd);
	return net_readnl (ms->fd);
}

void mcsdb_client_set (McSdbClient *ms, const char *key, const char *body, ut64 exptime UNUSED) {
	net_printf (ms->fd, "set %s 0 0 %d\r\n", key, strlen (body));
	net_flush (ms->fd);
	net_printf (ms->fd, "%s\r\n", body);
	net_flush (ms->fd);
	free (net_readnl (ms->fd));
}

static int sendcmd (int fd, const char *cmd UNUSED, const char *key, const char *body, ut64 exptime UNUSED) {
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

int mcsdb_client_add (McSdbClient *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "add", key, body, exptime);
}

int mcsdb_client_append (McSdbClient *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "append", key, body, exptime);
}

int mcsdb_client_prepend (McSdbClient *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "prepend", key, body, exptime);
}

int mcsdb_client_replace (McSdbClient *ms, const char *key, const char *body, ut64 exptime) {
	return sendcmd (ms->fd, "replace", key, body, exptime);
}

char *mcsdb_client_get (McSdbClient *ms, const char *key, ut64 *exptime) {
	int sexp, slen = 0;
	char *buf, *p;
	net_printf (ms->fd, "get %s\r\n", key);
	net_flush (ms->fd);
	buf = net_readnl (ms->fd);
	if (!buf)
		return NULL;

	buf[strlen (buf)]=0;
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
	if (buf) {
		int rv = read (ms->fd, buf, slen+2);
		buf[R_MAX(0,rv-1)] = 0;
	}
	free (net_readnl (ms->fd)); // read END
	return buf;
}

int mcsdb_client_remove (McSdbClient *ms, const char *key, ut64 exptime) {
	int ret;
	char *res;
	net_printf (ms->fd, "delete %s %lld 0\r\n", key, exptime);
	net_flush (ms->fd);
	res = net_readnl (ms->fd);
	ret = strstr (res, "NOT")? 0: 1;
	free (res);
	return ret;
}
