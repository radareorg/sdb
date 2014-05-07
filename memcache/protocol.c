/* mcsdb - LGPLv3 - Copyright 2011-2014 - pancake */

#include "mcsdb.h"
#include <ctype.h>
#include <sys/resource.h>

static inline void strtolower (char *s) {
	int i = 0;
	for (i=0; s[i]; i++)
		s[i] = tolower(s[i]);
}

static void handle_get(McSdb *ms, int fd, char *key, int smode) {
	ut64 exptime = 0LL;
	ut32 cas;
	int n = 0;
	char *s, *k, *K = key;
	if (!key) {
		net_printf (fd, "ERROR\r\n");
		return;
	}
	do {
		k = strchr (K, ' ');
		if (k) *k = 0;
		s = mcsdb_get (ms, K, &exptime, &cas);
		if (s) {
			if (smode) net_printf (fd, 
				"VALUE %s %llu %d %d\r\n",
				K, exptime, (int)strlen (s), cas);
			else net_printf (fd,
				"VALUE %s %llu %d\r\n", K, exptime, (int)strlen (s));
			net_printf (fd, "%s\r\n", s);
			free (s);
			n++;
//break;
		}
		if (k) K = k + 1;
	} while (k);
	net_printf (fd, "END\r\n"); // no elements found
}

int protocol_handle (McSdb *ms, McSdbClient *c, char *buf) {
	struct rusage ru;
	int fd, ret, reply, stored = 1;
	char *b, *p, *q, *cmd = buf, *key = NULL;
	int flags = 0, bytes = 0;
	ut64 n = 0;
	ut32 cmdhash = 0;

	if (!c || !*buf) { // never happenz
		//net_printf (fd, "ERROR\r\n");
		return 0;
	}
	fd = c->fd;
	//printf ("----> mode=%d buf=(%s)\n", c->mode, buf);
	if (c->mode == 1) {
		b = buf;
		b[c->len-1] = 0;
		switch (c->cmdhash) {
		case MCSDB_CMD_SET: mcsdb_set (ms, c->key, b, c->exptime, 0); break;
		case MCSDB_CMD_APPEND: mcsdb_append (ms, c->key, c->exptime, b); break;
		case MCSDB_CMD_ADD: stored = mcsdb_add (ms, c->key, c->exptime, b); break;
		case MCSDB_CMD_PREPEND: mcsdb_prepend (ms, c->key, c->exptime, b); break;
		case MCSDB_CMD_REPLACE: stored = mcsdb_replace (ms, c->key, c->exptime, b); break;
		default: break;
		}
		net_printf (fd, stored? "STORED\r\n": "NOT_STORED\r\n");
		c->mode = 0;
		c->idx = c->next;
		c->cmdhash = 0;
		return 1;
	}
	p = strchr (buf, ' ');
	if (p) {
		*p = 0;
		key = p + 1;
		if ((p=strchr (key, ' ')))
			*p = 0;
		strncpy (c->key, key, sizeof (c->key)-1); // XXX overflow
		if (p) {
			*p = ' ';
			p++;
		}
	}
	strtolower (cmd);

	cmdhash = sdb_hash (cmd);
	switch (cmdhash) {
	case MCSDB_CMD_FLUSH_ALL:
		mcsdb_flush (ms);
		net_printf (fd, "OK\r\n");
		break;
	case MCSDB_CMD_VERBOSITY:
		// not yet implemented
		net_printf (fd, "OK\r\n");
		break;
	case MCSDB_CMD_QUIT:
		return -1;
	case MCSDB_CMD_GET:
		if (key) // ignore error
			handle_get (ms, fd, key, 0);
		break;
	case MCSDB_CMD_GETS:
		handle_get (ms, fd, key, 1);
		break;
	case MCSDB_CMD_TOUCH: /* key exptime [noreply] */
		if (!key || !p) {
			net_printf (fd, "ERROR\r\n");
			return 0;
		}
		sscanf (p, "%llu", &n);
		net_printf (fd, mcsdb_touch (ms, key, n)?
			"TOUCHED\r\n": "NOT_TOUCHED\r\n");
		break;
	case MCSDB_CMD_INCR:
	case MCSDB_CMD_DECR:
		if (!key || !p) {
			net_printf (fd, "ERROR\r\n");
			return 0;
		}
		n = strtoull (p, &q, 10);
		if (q) {
			p--; *p=0;
			if (cmdhash==MCSDB_CMD_INCR)
				p = mcsdb_incr (ms, key, n);
			else p = mcsdb_decr (ms, key, n);
			if (p) {
				net_printf (fd, "%s\r\n", p);
				free (p);
			} else net_printf (fd, "NOT_FOUND\r\n");
		} else net_printf (fd, "ERROR\r\n");
		break;
	case MCSDB_CMD_STATS:
		getrusage (0, &ru);
		net_printf (fd, "STAT pid %d\r\n", getpid ());
		net_printf (fd, "STAT version "MCSDB_VERSION"\r\n");
		net_printf (fd, "STAT pointer_size %u\r\n", (int)sizeof (void*)*8);
		net_printf (fd, "STAT time %llu\r\n", sdb_now ());
		net_printf (fd, "STAT uptime %llu\r\n", sdb_now ()-ms->time);
		net_printf (fd, "STAT rusage_user %u.%u\r\n",
			(ut32)ru.ru_utime.tv_sec, (ut32)ru.ru_utime.tv_usec);
		net_printf (fd, "STAT rusage_system %u.%u\r\n",
			(ut32)ru.ru_stime.tv_sec, (ut32)ru.ru_stime.tv_usec);
		net_printf (fd, "STAT curr_connections %u\r\n", ms->nfds-1);
		net_printf (fd, "STAT total_connections %u\r\n", ms->tfds);
		net_printf (fd, "STAT cmd_get %llu\r\n", ms->gets);
		net_printf (fd, "STAT cmd_set %llu\r\n", ms->sets);
		net_printf (fd, "STAT get_hits %llu\r\n", ms->hits);
		net_printf (fd, "STAT get_misses %llu\r\n", ms->misses);
		net_printf (fd, "STAT evictions %llu\r\n", ms->evictions);
		net_printf (fd, "STAT bytes_read %llu\r\n", ms->bread);
		net_printf (fd, "STAT bytes_written %llu\r\n", ms->bwrite);
		// ?? net_printf (fd, "STAT limit_maxbytes 0\r\n");
		net_printf (fd, "STAT threads 1\r\n");
		net_printf (fd, "END\r\n");
		break;
	case MCSDB_CMD_VERSION:
		net_printf (fd, "VERSION 0.1\r\n");
		break;
	case MCSDB_CMD_DELETE:
		if (!key) return 0;
		p = strchr (key, ' ');
		if (p) {
			*p = 0;
			if (!memcmp (p+1, "noreply", 7))
				reply = 0;
			else reply = 1;
		} else reply = 1;
		c->exptime = 0LL;
		sscanf (key, "%llu", &c->exptime);
		ret = mcsdb_remove (ms, key, c->exptime);
		if (reply) {
			if (ret) net_printf (fd, "DELETED\r\n");
			else net_printf (fd, "NOT_FOUND\r\n");
		}
		break;
	case MCSDB_CMD_ADD:
	case MCSDB_CMD_SET:
	case MCSDB_CMD_APPEND:
	case MCSDB_CMD_PREPEND:
	case MCSDB_CMD_REPLACE:
		if (!key || !p) {// || !((p=strchr (key, ' ')))) {
			net_printf (fd, "ERROR\r\n");
			return 0;
		}
		ret = sscanf (p, "%d %llu %d", &flags, &c->exptime, &bytes);
		if (ret != 3) {
			net_printf (fd, "ERROR parsing (%s)\r\n", p+1);
			return 0;
		}
		if (bytes<1) {
			net_printf (fd, "CLIENT_ERROR invalid length\r\n");
			net_printf (fd, "ERROR\r\n");
			return 0;
		}
		c->mode = 1; // read N bytes
		//c->next = 0;
		c->idx = c->next;
		c->len = bytes+1;
		c->cmdhash = cmdhash;
		// continue on mode 1
		return 1;
	case MCSDB_CMD_CAS:
		// check and set
		// cas <key> <flags> <exptime> <bytes> <cas unqiue> [noreply]\r\n
		printf ("NOT YET IMPLEMENTED\n");
		return 0;
	default:
		net_printf (fd, "ERROR\r\n");
	}
	return 0;
}
