#ifndef _INCLUDE_SDB_MEMCACHE_H_
#define _INCLUDE_SDB_MEMCACHE_H_

#include "sdb.h"
#include <poll.h>

/* "mcsdb.sdb" */
#define MEMCACHE_FILE NULL
#define MEMCACHE_PORT 11211
#define MEMCACHE_VERSION "0.1"
#define MEMCACHE_MAX_CLIENTS 16
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
} McSdbClient;

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
	McSdbClient *msc[MEMCACHE_MAX_CLIENTS+1];
} McSdb;

extern McSdb *ms;

McSdb *mcsdb_new (const char *file);
void mcsdb_free (McSdb *ms);
void mcsdb_set(McSdb *ms, const char *key, ut64 exptime, const char *body);
char *mcsdb_get (McSdb *ms, const char *key, ut64 *exptime);
char *mcsdb_incr(McSdb *ms, const char *key, ut64 val);
char *mcsdb_decr(McSdb *ms, const char *key, ut64 val);
int mcsdb_replace(McSdb *ms, const char *key, ut64 exptime, const char *body);
int mcsdb_delete(McSdb *ms, const char *key, ut64 exptime);
int mcsdb_add(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_append(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_prepend(McSdb *ms, const char *key, ut64 exptime, const char *body);

int net_listen (int port);
int net_close (int s);
int net_flush(int fd);
int net_printf(int fd, char *fmt, ...);

#endif
