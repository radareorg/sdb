#ifndef _INCLUDE_SDB_H_
#define _INCLUDE_SDB_H_

#include "ht.h"
#include "list.h"
#include "cdb.h"
#include "cdb_make.h"

typedef struct sdb_t {
	char *dir;
	int fd;
	int lock;
	struct cdb db;
	struct cdb_make m;
	RHashTable *ht;
	ut32 eod;
} sdb;

#define SDB_BLOCK 4096
#define SDB_KEYSIZE 32
#define SDB_VALUESIZE (SDB_BLOCK-sizeof(ut64)-SDB_KEYSIZE)

typedef struct sdb_kv {
	char key[SDB_KEYSIZE];
	char value[SDB_VALUESIZE];
	ut64 expire;
} SdbKv;

sdb *sdb_new (const char *dir, int lock);
int sdb_exists (sdb*, const char *key);
int sdb_nexists (sdb*, const char *key);
int sdb_delete (sdb*, const char *key);
char *sdb_get (sdb*, const char *key);
int sdb_set (sdb*, const char *key, const char *data);
void sdb_list(sdb*);
int sdb_sync (sdb*);
void sdb_kv_free (struct sdb_kv *kv);
void sdb_dump_begin (sdb* s);
int sdb_add (struct cdb_make *c, const char *key, const char *data);
int sdb_dump_next (sdb* s, char *key, char *value);
void sdb_free (sdb* s);

ut64 sdb_getn (sdb *s, const char *key);
void sdb_setn (sdb *s, const char *key, ut64 v);
ut64 sdb_inc (sdb *s, const char *key, ut64 n);
ut64 sdb_dec (sdb *s, const char *key, ut64 n);

int sdb_lock(const char *s);
const char *sdb_lockfile(const char *f);
void sdb_unlock(const char *s);
int sdb_expire(sdb *s, const char *key, ut64 expire);
ut64 sdb_get_expire(sdb *s, const char *key);
ut64 sdb_now ();
ut32 sdb_hash ();

#endif
