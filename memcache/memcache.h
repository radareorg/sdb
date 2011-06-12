#ifndef _INCLUDE_SDB_MCSDB_H_
#define _INCLUDE_SDB_MCSDB_H_

#include "sdb.h"
#include <poll.h>

//#define printf // 
/* "mcsdb.sdb" */
#define MCSDB_FILE NULL
#define MCSDB_PORT 11211
#define MCSDB_VERSION "0.1"
#define MCSDB_MAX_CLIENTS 16
#define MCSDB_MAX_BUFFER 4096

#include "cmds.h"

#if 0
 buf                       idx
  |_____.______|______._____|
#endif

typedef struct {
	int fd;
	int mode;
	int len; // bytes to read (must be removed?)
	int idx; // number of bytes writte in buf
	int next; // use this length for next read
	ut32 cmdhash;
	char buf[MCSDB_MAX_BUFFER]; // buffer
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
	struct pollfd fds[MCSDB_MAX_CLIENTS+1];
	int nfds;
	int tfds; // total number of clients
	McSdbClient *msc[MCSDB_MAX_CLIENTS+1];
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

int net_connect(const char *host, const char *port);
int net_listen (int port);
int net_close (int s);
int net_flush(int fd);
int net_printf(int fd, char *fmt, ...);
char *net_readnl(int fd);

/* client */
char *mcsdb_client_incr(McSdb *ms, const char *key, ut64 val);
char *mcsdb_client_decr(McSdb *ms, const char *key, ut64 val);
void mcsdb_client_set(McSdb *ms, const char *key, ut64 exptime, const char *body);
int mcsdb_client_add(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_client_append(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_client_prepend(McSdb *ms, const char *key, ut64 exptime, const char *body);
int mcsdb_client_replace(McSdb *ms, const char *key, ut64 exptime, const char *body);
char *mcsdb_client_get (McSdb *ms, const char *key, ut64 *exptime);
int mcsdb_client_delete(McSdb *ms, const char *key, ut64 exptime);

#endif
