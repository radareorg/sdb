/* Copyleft 2011-2014 - mcsdb (aka memcache-SimpleDB) - pancake */

#include "mcsdb.h"

McSdb *mcsdb_new(const char *file) {
	McSdb *ms;
	Sdb *s = sdb_new (NULL, file, 0);
	if (!s) return NULL;
	ms = R_NEW (McSdb);
	if (!ms) {
		sdb_free (s);
		return NULL;
	}
	ms->sdb = s;
	ms->time = sdb_now ();
	ms->gets = ms->sets = 0LL;
	ms->evictions = ms->hits = ms->misses = 0LL;
	ms->bread = ms->bwrite = 0LL;
	ms->nfds = ms->tfds = 0;
	return ms;
}

void mcsdb_flush(McSdb *ms) {
	sdb_reset (ms->sdb);
}

void mcsdb_free(McSdb *ms) {
	int i;
	for (i=0; i<ms->nfds; i++)
		net_close (ms->fds[i].fd);
	sdb_sync (ms->sdb);
	sdb_free (ms->sdb);
	free (ms);
}

char *mcsdb_incr(McSdb *ms, const char *key, ut64 val) {
	if (!sdb_num_exists (ms->sdb, key))
		return NULL;
	if (sdb_num_inc (ms->sdb, key, val, 0) == 0LL)
		return NULL;
	ms->sets++;
	return sdb_get (ms->sdb, key, 0);
}

char *mcsdb_decr(McSdb *ms, const char *key, ut64 val) {
	if (!sdb_num_exists (ms->sdb, key))
		return NULL;
	sdb_num_dec (ms->sdb, key, val, 0);
	ms->sets++;
	return sdb_get (ms->sdb, key, 0);
}

int mcsdb_set(McSdb *ms, const char *key, const char *value, ut64 exptime, ut32 cas) {
	int ret = sdb_set (ms->sdb, key, value, cas);
	sdb_expire_set (ms->sdb, key, exptime, cas);
	ms->sets++;
	return ret;
}

int mcsdb_add(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	if (!sdb_exists (ms->sdb, key)) {
		sdb_set (ms->sdb, key, body, 0);
		sdb_expire_set (ms->sdb, key, exptime, 0);
		ms->sets++;
		ms->hits++;
		return 1;
	}
	ms->misses++;
	return 0;
}

void mcsdb_append(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	int len = strlen (body);
	char *a, *b;
	a = sdb_get (ms->sdb, key, NULL);
	if (a) {
		int alen = strlen (a);
		b = malloc (1 + len + alen);
		memcpy (b, a, alen);
		strcpy (b+alen, body);
		sdb_set (ms->sdb, key, b, 0);
		free (b);
		free (a);
	} else sdb_set (ms->sdb, key, body, 0);
	sdb_expire_set (ms->sdb, key, exptime, 0);
	ms->sets++;
}

void mcsdb_prepend(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	int len = strlen (body);
	char *a, *b;
	a = sdb_get (ms->sdb, key, 0);
	if (a) {
		int alen = strlen (a);
		b = malloc (1 + len + alen);
		strcpy (b, body);
		strcpy (b+len, a);
		sdb_set (ms->sdb, key, b, 0);
		free (b);
		free (a);
	} else sdb_set (ms->sdb, key, body, 0);
	sdb_expire_set (ms->sdb, key, exptime, 0);
	ms->sets++;
}

int mcsdb_replace(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	if (sdb_exists (ms->sdb, key)) {
		sdb_set (ms->sdb, key, body, 0);
		sdb_expire_set (ms->sdb, key, exptime, 0);
		ms->sets++;
		ms->hits++;
		return 1;
	}
	ms->misses++;
	return 0;
}

/*
  NOT IMPLEMENTED
  "cas" is a check and set operation which means "store this data but
  only if no one else has updated since I last fetched it."
*/
// cas <key> <flags> <exptime> <bytes> <cas unqiue> [noreply]\r\n
int mcsdb_cas(McSdb *ms, const char *key, const char *value, ut64 exptime, ut32 cas) {
	return mcsdb_set (ms, key, value, exptime, cas);
}

/* retrieval */
char *mcsdb_get(McSdb *ms, const char *key, ut64 *exptime, ut32 *cas) {
	ut32 n;
	char *s = sdb_get (ms->sdb, key, &n);
	if (s) ms->hits++;
	else ms->misses++;
	ms->gets++;
	if (cas) *cas = n;
	*exptime = sdb_expire_get (ms->sdb, key, NULL);
	return s;
}

int mcsdb_remove(McSdb *ms, const char *key, ut64 exptime) {
	if (!sdb_exists (ms->sdb, key))
		return 0;
	if (exptime>0) {
		sdb_expire_set (ms->sdb, key, exptime, 0);
		return 0;
	}
	if (sdb_expire_get (ms->sdb, key, NULL)>0)
		ms->evictions++;
	return sdb_unset (ms->sdb, key, 0);
}

int mcsdb_touch(McSdb *ms, const char *key, ut64 exptime) {
	if (!sdb_exists (ms->sdb, key))
		return 0;
	return sdb_expire_set (ms->sdb, key, exptime, 0);
}
