#ifndef R_PAIR_H
#define R_PAIR_H

#include <r_util.h>

#ifdef __cplusplus
extern "C" {
#endif
// TODO: add support for network. (udp). memcache, with hooks
typedef struct r_pair_t {
	char *dir;
	char *file;
	void *sdb;
	Ht *ht;
	RList *dbs;
} RPair;

typedef struct r_pair_item_t {
	char *k, *v;
} RPairItem;

#ifdef R_API
R_API RPairItem *r_pair_item_new(const char *k, const char *v);
R_API void r_pair_item_free(RPairItem*);

R_API int r_pair_load(RPair *p, const char *f);
R_API int r_pair_save(RPair *p, const char *f);
R_API RPair *r_pair_new(void);
R_API RPair *r_pair_new_from_file(const char *file);
R_API void r_pair_free(RPair *p);
R_API void r_pair_delete(RPair *p, const char *name);
R_API char *r_pair_get(RPair *p, const char *name);
R_API void r_pair_set(RPair *p, const char *name, const char *value);
R_API RList *r_pair_list(RPair *p, const char *domain);
R_API void r_pair_set_sync_dir(RPair *p, const char *dir);
R_API int r_pair_sync(RPair *p);
R_API void r_pair_reset(RPair *p);
#endif

#ifdef __cplusplus
}
#endif

#endif
