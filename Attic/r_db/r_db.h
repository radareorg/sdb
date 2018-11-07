#ifndef R2_DB_H
#define R2_DB_H

#include "r_types.h"
#include "r_util.h"
#include "sdb.h"

#ifdef __cplusplus
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_db);


#define R_DB_KEYS 256

typedef struct r_db_block_t {
	void **data; /* { 0x80380, 0x80380, 0 } */
	struct r_db_block_t *childs[256];
} RDatabaseBlock;

#define R_DB_INDEXOF(type, member) \
  (int)((size_t)((&((type *)0)->member)))

typedef struct r_db_t {
	int id_min;
	int id_max;
	RDatabaseBlock *blocks[R_DB_KEYS];
	int blocks_sz[R_DB_KEYS];
	void *cb_user;
	int (*cb_free)(void *db, const void *item, void *user);
} RDatabase;

typedef struct r_db_iter_t {
	RDatabase *db;
	int key;       /* key to be used */
	int size;      /* key size */
	int path[256]; /* for each depth level */
	int ptr;       /* pointer in block nodes (repeated childs) */
	void *cur;
} RDatabaseIter;

/* table */
typedef struct r_db_table_t {
	char *name;
	int nelems;
	char *fmt;
	char *args;
	int *offset;
} RDatabaseTable;

#if 0
it = r_db_iterator (db);
while (r_db_iter_next(it)) {
	f = (RAnalFcn*) r_db_iter_get (it);
	/* ... */
}
#endif

#ifdef R_API
R_API RDatabase *r_db_new(void);
R_API RDatabaseBlock *r_db_block_new(void);
R_API int r_db_add_id(RDatabase *db, int off, int size);
R_API int r_db_add(RDatabase *db, void *b);
R_API int r_db_add_unique(RDatabase *db, void *b);
R_API void **r_db_get(RDatabase *db, int key, const ut8 *b);
R_API void *r_db_get_cur(void **ptr);
R_API bool r_db_delete(RDatabase *db, const void *b);
R_API void **r_db_get_next(void **ptr);
R_API RDatabaseIter *r_db_iter(RDatabase *db, int key, const ut8 *b);
R_API void *r_db_iter_cur(RDatabaseIter *iter);
R_API int r_db_iter_next(RDatabaseIter *iter);
R_API void *r_db_iter_prev(RDatabaseIter *iter);
R_API RDatabaseIter *r_db_iter_new(RDatabase *db, int key);
R_API RDatabaseIter *r_db_iter_free(RDatabaseIter *iter);
R_API int r_db_free(RDatabase *db);
R_API void *r_db_table_free(struct r_db_table_t *table);


//R_API int r_db_push(RDatabase *db, const ut8 *b);
//R_API ut8 *r_db_pop(RDatabase *db);
#endif
#ifdef __cplusplus
}
#endif

#endif
