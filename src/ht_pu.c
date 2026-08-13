/* sdb - MIT - Copyright 2018-2026 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_pu.h"
#include "sdb/heap.h"
#include "sdb/cwisstable.h"

CWISS_DECLARE_FLAT_HASHMAP_DEFAULT(HtPU_, void*, ut64);

struct HtPU_t {
	HtPU_ inner;
};

#define HT HtPU
#define HT_(n) ht_pu_##n
#define CW_(n) HtPU__##n
#define HT_KEY void *
#define HT_CB HtPUForEachCallback
#define HT_KEY_SAME(a, b) ((a) == (b))
#include "ht_cwiss.inc.c"
