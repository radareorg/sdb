#ifndef _INCLUDE_SDB_MEMCACHE_H_
#define _INCLUDE_SDB_MEMCACHE_H_

#include "sdb.h"
#include <poll.h>

/* "mcsdb.sdb" */
#define MEMCACHE_FILE NULL
#define MEMCACHE_PORT 11211
#define MEMCACHE_VERSION "0.1"
#define MEMCACHE_MAX_CLIENTS 1
#define MEMCACHE_MAX_BUFFER 1024

#include "cmds.h"

typedef struct {
	int fd;
	int mode;
	int len; // bytes to read
	int idx; // bytes readed
	ut32 cmdhash;
	char buf[MEMCACHE_MAX_BUFFER]; // buffer
	ut64 exptime;
	char key[100];
} MemcacheSdbClient;

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
	MemcacheSdbClient *msc[MEMCACHE_MAX_CLIENTS+1];
} MemcacheSdb;

extern MemcacheSdb *ms;

MemcacheSdb *mcsdb_new (const char *file);
void mcsdb_free (MemcacheSdb *ms);
void memcache_set(MemcacheSdb *ms, const char *key, ut64 exptime, const char *body);
char *memcache_get (MemcacheSdb *ms, const char *key, ut64 *exptime);
char *memcache_incr(MemcacheSdb *ms, const char *key, ut64 val);
char *memcache_decr(MemcacheSdb *ms, const char *key, ut64 val);
int memcache_replace(MemcacheSdb *ms, const char *key, ut64 exptime, const char *body);
int memcache_delete(MemcacheSdb *ms, const char *key, ut64 exptime);
int memcache_add(MemcacheSdb *ms, const char *key, ut64 exptime, const char *body);
void memcache_append(MemcacheSdb *ms, const char *key, ut64 exptime, const char *body);
void memcache_prepend(MemcacheSdb *ms, const char *key, ut64 exptime, const char *body);

int net_listen (int port);
int net_close (int s);
void net_flush(int fd);
int net_printf(int fd, char *fmt, ...);

#endif
