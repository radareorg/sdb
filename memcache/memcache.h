#ifndef _INCLUDE_SDB_MEMCACHE_H_
#define _INCLUDE_SDB_MEMCACHE_H_

#include "sdb.h"
#include <poll.h>

/* "mcsdb.sdb" */
#define MEMCACHE_FILE NULL
#define MEMCACHE_PORT 11211
#define MEMCACHE_VERSION "0.1"
#define MEMCACHE_MAX_CLIENTS 10

typedef struct {
	sdb *sdb;
	int fd;
	/* stats */
	ut64 time;
	ut64 gets;
	ut64 sets;
	ut64 hits;
	ut64 misses;
	ut64 evictions;
	ut64 bread;
	ut64 bwrite;
	/* network */
	struct pollfd fds[MEMCACHE_MAX_CLIENTS+1];
	int nfds;
	int tfds; // total number of clients
} MemcacheSdb;

typedef struct {
	int fd;
	MemcacheSdb *ms;
} MemcacheSdbClient;

extern MemcacheSdb *ms;

MemcacheSdb *mcsdb_new (const char *file);
void mcsdb_free (MemcacheSdb *ms);
void memcache_set(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body);
char *memcache_get (MemcacheSdb *ms, const char *key, ut64 *exptime);
char *memcache_incr(MemcacheSdb *ms, const char *key, ut64 val);
char *memcache_decr(MemcacheSdb *ms, const char *key, ut64 val);
int memcache_replace(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body);
int memcache_delete(MemcacheSdb *ms, const char *key, ut64 exptime);
int memcache_add(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body);
void memcache_append(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body);
void memcache_prepend(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body);

#endif
