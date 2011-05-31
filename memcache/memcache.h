#include "sdb.h"

#define MEMCACHE_FILE "mcsdb.sdb"
#define MEMCACHE_PORT 11211
#define MEMCACHE_VERSION "0.1"

typedef struct {
	sdb *sdb;
	int fd;
} MemcacheSdb;

typedef struct {
	MemcacheSdb *ms;
} MemcacheSdbClient;


MemcacheSdb *memcache_sdb_new (const char *file);
void memcache_free (MemcacheSdb *ms);
void memcache_set(MemcacheSdb *ms, const char *key, ut64 exptime, const ut8 *body, int len);
char *memcache_get (MemcacheSdb *ms, const char *key, ut64 *exptime);
