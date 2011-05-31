#include "memcache.h"

MemcacheSdb *memcache_sdb_new (const char *file) {
	MemcacheSdb *ms;
	sdb *s = sdb_new (file, R_FALSE);
	if (!s) return NULL;
	ms = R_NEW (MemcacheSdb);
	if (!ms) {
		sdb_free (s);
		return NULL;
	}
	ms->sdb = s;
	return ms;
}

void memcache_free (MemcacheSdb *ms) {
	sdb_sync (ms->sdb);
	sdb_free (ms->sdb);
	free (ms);
}

/* storage */
void memcache_incr();
void memcache_decr();
void memcache_set(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body, int len) {
	sdb_set (ms->sdb, key, body);
}
void memcache_add();
void memcache_replace();
void memcache_append();
void memcache_prepend();
/*
  "cas" is a check and set operation which means "store this data but
  only if no one else has updated since I last fetched it."
*/
void memcache_cas();

/* retrieval */
char *memcache_get (MemcacheSdb *ms, const char *key, ut64 *exptime) {
	char *s = sdb_get (ms->sdb, key);
	*exptime = 0;
	return s;
}
void memcache_gets ();
void memcache_delete();

/* other */
void memcache_stats();
void memcache_version();
void memcache_quit ();
