/* sdb - MIT - Copyright 2018-2026 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_uu.h"
#include "sdb/heap.h"
#include "sdb/cwisstable.h"

typedef uint64_t ut64;

CWISS_DECLARE_FLAT_HASHMAP_DEFAULT(HtUU_, ut64, ut64);

struct HtUU_t {
	HtUU_ inner;
};

#define HT HtUU
#define HT_(n) ht_uu_##n
#define CW_(n) HtUU__##n
#define HT_KEY const ut64
#define HT_CB HtUUForEachCallback
#define HT_KEY_SAME(a, b) ((a) == (b))
#include "ht_cwiss.inc.c"
