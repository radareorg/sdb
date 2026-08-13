/* sdb - MIT - Copyright 2018-2026 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_su.h"
#include "sdb/heap.h"
#include "sdb/sdb.h"
#include "sdb/cwisstable.h"

static inline void string_copy(void *dst, const void *src);
static inline void string_dtor(void *val);
static inline size_t string_hash(const void *val);
static inline bool string_eq(const void *a, const void *b);

CWISS_DECLARE_FLAT_HASHMAP(HtSU_, char*, ut64, string_copy, string_dtor, string_hash, string_eq);

struct HtSU_t {
	HtSU_ inner;
};

static inline void string_copy(void *dst_, const void *src_) {
	const HtSU__Entry *src = (const HtSU__Entry *)src_;
	HtSU__Entry *dst = (HtSU__Entry *)dst_;
	dst->key = sdb_strdup (src->key);
	dst->val = src->val;
}

static inline void string_dtor(void *val) {
	char *str = *(char**)val;
	sdb_gh_free (str);
}

static inline size_t string_hash(const void *val) {
	const char *str = *(const char *const *)val;
	CWISS_FxHash_State state = 0;
	CWISS_FxHash_Write (&state, str, strlen (str));
	return state;
}

static inline bool string_eq(const void *a, const void *b) {
	const char *ap = *(const char* const *)a;
	const char *bp = *(const char* const *)b;
	return strcmp (ap, bp) == 0;
}

#define HT HtSU
#undef HT_ // clash with the ht_inc.h template pulled in via sdb.h
#define HT_(n) ht_su_##n
#define CW_(n) HtSU__##n
#define HT_KEY const char *
#define HT_CB HtSUForEachCallback
#define HT_KEY_SAME(a, b) (strcmp (a, b) == 0)
#include "ht_cwiss.inc.c"
