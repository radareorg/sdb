#ifndef _INCLUDE_SDB_MCSDB_H_
#define _INCLUDE_SDB_MCSDB_H_

#include "sdb.h"
#if !__SDB_WINDOWS__
#include <poll.h>
#endif

/* "mcsdb.sdb" */
#define MCSDB_FILE NULL
#define MCSDB_PORT 11211
#define MCSDB_VERSION "0.2"
#define MCSDB_MAX_CLIENTS 16
#define MCSDB_MAX_BUFFER 4096

#include "cmds.h"

typedef struct {
	int fd;
	int mode;
	int len; // bytes to read (must be removed?)
	int idx; // number of bytes writte in buf
	int next; // use this length for next read
	ut32 cmdhash;
	char buf[MCSDB_MAX_BUFFER]; // buffer
	ut64 exptime;
	char key[256];
} McSdbClient;

typedef struct {
	Sdb *sdb;
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

#define NETBUFSZ 1024
typedef struct {
	int fd;
	int len;
	ut8 buf[NETBUFSZ];
} NetClient;

int protocol_handle(McSdb *, McSdbClient *c, char *buf);

McSdb *mcsdb_new(const char *file);
void mcsdb_free(McSdb *ms);
void mcsdb_flush(McSdb *ms);
int mcsdb_set(McSdb *ms, const char *key, const char *value, ut64 exptime, ut32 cas);
int mcsdb_cas(McSdb *ms, const char *key, const char *value, ut64 exptime, ut32 cas);
int mcsdb_touch(McSdb *ms, const char *key, ut64 exptime);
char *mcsdb_get(McSdb *ms, const char *key, ut64 *exptime, ut32 *cas);
char *mcsdb_incr(McSdb *ms, const char *key, ut64 val);
char *mcsdb_decr(McSdb *ms, const char *key, ut64 val);
int mcsdb_replace(McSdb *ms, const char *key, ut64 exptime, const char *body);
int mcsdb_remove(McSdb *ms, const char *key, ut64 exptime);
int mcsdb_add(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_append(McSdb *ms, const char *key, ut64 exptime, const char *body);
void mcsdb_prepend(McSdb *ms, const char *key, ut64 exptime, const char *body);

int net_connect(const char *host, const char *port);
int net_listen(int port);
int net_close(int s);
int net_flush(int fd);
int net_printf(int fd, const char *fmt, ...);
char *net_readnl(int fd);

/* client */
McSdbClient *mcsdb_client_new(const char *host, const char *port);
void mcsdb_client_free(McSdbClient *);
char *mcsdb_client_incr(McSdbClient *ms, const char *key, ut64 val);
char *mcsdb_client_decr(McSdbClient *ms, const char *key, ut64 val);
void mcsdb_client_set(McSdbClient *ms, const char *key, const char *body, ut64 exptime);
int mcsdb_client_add(McSdbClient *ms, const char *key, const char *body, ut64 exptime);
int mcsdb_client_append(McSdbClient *ms, const char *key, const char *body, ut64 exptime);
int mcsdb_client_prepend(McSdbClient *ms, const char *key, const char *body, ut64 exptime);
int mcsdb_client_replace(McSdbClient *ms, const char *key, const char *body, ut64 exptime);
char *mcsdb_client_get (McSdbClient *ms, const char *key, ut64 *exptime);
int mcsdb_client_remove(McSdbClient *ms, const char *key, ut64 exptime);

#endif
