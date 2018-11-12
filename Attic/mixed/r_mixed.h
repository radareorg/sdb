#ifndef R_MIXED_H
#define R_MIXED_H
#include <r_list.h>
#include <sdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define R_MIXED_KEY(m,x,y,z) r_mixed_key(m, r_offsetof(x,z), sizeof(y->z))
#define RMIXED_MAXKEYS 256
typedef struct r_mixed_data_t {
	int size;
	union {
		Ht *ht;
		Ht *ht64;
	} hash;
} RMixedData;

typedef struct r_mixed_t {
	RList *list;
	RMixedData *keys[RMIXED_MAXKEYS];
	ut64 state[RMIXED_MAXKEYS]; // used by change_(begin|end)
} RMixed;

R_API RMixed *r_mixed_new ();
R_API void r_mixed_free (RMixed *m);
R_API int r_mixed_key_check(RMixed *m, int key, int sz);
R_API int r_mixed_key(RMixed *m, int key, int size);
R_API ut64 r_mixed_get_value(int key, int sz, const void *p);
R_API RList *r_mixed_get (RMixed *m, int key, ut64 value);
R_API void *r_mixed_get0 (RMixed *m, int key, ut64 value);
R_API int r_mixed_add(RMixed *m, void *p);
R_API int r_mixed_del (RMixed *m, void *p);
R_API void r_mixed_change_begin(RMixed *m, void *p);
R_API bool r_mixed_change_end(RMixed *m, void *p);

#ifdef __cplusplus
}
#endif

#endif //  R_MIXED_H
