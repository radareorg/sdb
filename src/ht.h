/* radare2 - BSD 3 Clause License - 2016 - crowell */

#ifndef __HT_H
#define __HT_H

#include "ls.h"
#include "types.h"

typedef struct ht_kv {
	ut64 key;
	ut64 value;
	ut32 key_len;
	ut32 value_len;
} HtKv;

typedef void (*HtKvFreeFunc)(HtKv *);
typedef ut64 (*DupKey)(ut64);
typedef ut64 (*DupValue)(ut64);
typedef void *(*PDupKey)(const void *);
typedef void *(*PDupValue)(const void *);
typedef size_t (*CalcSize)(ut64);
typedef size_t (*PCalcSize)(const void *);
typedef ut32 (*HashFunction)(ut64);
typedef ut32 (*PHashFunction)(const void *);
typedef int (*ListComparator)(ut64, ut64);
typedef int (*PListComparator)(const void *, const void *);
typedef bool (*HtForeachCallback)(void *user, ut64, ut64);
typedef bool (*HtPForeachCallback)(void *user, const void *, void *);
typedef bool (*HtUForeachCallback)(void *user, ut64, void *);

typedef struct ht_bucket_t {
	HtKv *arr;
	ut32 count;
} HtBucket;

typedef enum {
      // regular ht with ut64 as both keys and values
      HT_TYPE_NONE = 1,
      // ht with pointers as both keys and values
      HT_TYPE_P,
      // ht with ut64 as keys and pointers as values
      HT_TYPE_U,
} HtType;

typedef struct ht_n_options_t {
	ListComparator cmp;   	// Function for comparing values. Returns 0 if eq.
	HashFunction hashfn;  	// Function for hashing items in the hash table.
	DupKey dupkey;  	// Function for making a copy of key
	DupValue dupvalue;  	// Function for making a copy of value
	CalcSize calcsizeK;     // Function to determine the key's size
	CalcSize calcsizeV;  	// Function to determine the value's size
} HtNOptions;

typedef struct ht_p_options_t {
	PListComparator cmp;   	// Function for comparing values. Returns 0 if eq.
	PHashFunction hashfn;  	// Function for hashing items in the hash table.
	PDupKey dupkey;  	// Function for making a copy of key
	PDupValue dupvalue;  	// Function for making a copy of value
	PCalcSize calcsizeK;     // Function to determine the key's size
	PCalcSize calcsizeV;  	// Function to determine the value's size
} HtPOptions;

typedef struct ht_u_options_t {
	ListComparator cmp;   	// Function for comparing values. Returns 0 if eq.
	HashFunction hashfn;  	// Function for hashing items in the hash table.
	DupKey dupkey;  	// Function for making a copy of key
	PDupValue dupvalue;  	// Function for making a copy of value
	CalcSize calcsizeK;     // Function to determine the key's size
	PCalcSize calcsizeV;  	// Function to determine the value's size
} HtUOptions;

typedef struct ht_options_t {
	HtType type;	  // Type of the ht (if 0 it will be HT_TYPE_NONE by default)
	HtKvFreeFunc freefn;  	// Function to free the keyvalue store
	size_t elem_size;       // Size of each HtKv element (useful for subclassing like SdbKv)
	union {
		HtNOptions n;
		HtPOptions p;
		HtUOptions u;
	} sub;
} HtOptions;

/** ht **/
typedef struct ht_t {
	ut32 size;	  // size of the hash table in buckets.
	ut32 count;	  // number of stored elements.
	HtBucket* table;  // Actual table.
	ut32 prime_idx;
	HtOptions opt;
} SdbHt;

SDB_API HtPOptions *ht_p_options_init(HtOptions *opt, HtKvFreeFunc freefn);
SDB_API HtUOptions *ht_u_options_init(HtOptions *opt, HtKvFreeFunc freefn);
SDB_API HtNOptions *ht_n_options_init(HtOptions *opt, HtKvFreeFunc freefn);

// Create a new RHashTable.
// If hashfunction is NULL it will be used sdb_hash internally
// If keydup or valdup are null it will be used an assignment
// If keySize or valueSize are null it will be used strlen internally
SDB_API SdbHt* ht_new_opt(HtOptions *opt);
// Destroy a hashtable and all of its entries.
SDB_API void ht_free(SdbHt* ht);
// Insert a new Key-Value pair into the hashtable. If the key already exists, returns false.
SDB_API bool ht_insert(SdbHt* ht, ut64 key, ut64 value);
// Insert a new Key-Value pair into the hashtable, or updates the value if the key already exists.
SDB_API bool ht_update(SdbHt* ht, ut64 key, ut64 value);
// Delete a key from the hashtable.
SDB_API bool ht_delete(SdbHt* ht, ut64 key);
// Find the value corresponding to the matching key.
SDB_API ut64 ht_find(SdbHt* ht, ut64 key, bool* found);
SDB_API void ht_foreach(SdbHt *ht, HtForeachCallback cb, void *user);

SDB_API HtKv* ht_find_kv(SdbHt* ht, ut64 key, bool* found);
SDB_API bool ht_insert_kv(SdbHt *ht, HtKv *kv, bool update);

static inline void *ht_u2ptr(ut64 val) {
	return (void *)(uintptr_t)val;
}

static inline ut64 ht_ptr2u(void *val) {
	return (ut64)(uintptr_t)val;
}

// pointer API
// Useful when the keys are pointers
SDB_API SdbHt* ht_new0_p();
SDB_API SdbHt* ht_new_p(PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize valueSize);
SDB_API SdbHt* ht_new_p_size(ut32 initial_size, PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize valueSize);
SDB_API void ht_foreach_p(SdbHt *ht, HtPForeachCallback cb, void *user);

static inline bool ht_insert_p(SdbHt* ht, const void* key, void* value) {
	return ht_insert (ht, (ut64)(uintptr_t)key, (ut64)(uintptr_t)value);
}

static inline bool ht_update_p(SdbHt* ht, const void* key, void* value) {
	return ht_update (ht, (ut64)(uintptr_t)key, (ut64)(uintptr_t)value);
}

static inline bool ht_delete_p(SdbHt* ht, const void* key) {
	return ht_delete (ht, (ut64)(uintptr_t)key);
}

static inline void* ht_find_p(SdbHt* ht, const void* key, bool* found) {
	return (void *)(uintptr_t)ht_find (ht, (ut64)(uintptr_t)key, found);
}

static inline void* ht_kv_pkey(HtKv *kv) {
	return ht_u2ptr(kv->key);
}

static inline void* ht_kv_pval(HtKv *kv) {
	return ht_u2ptr(kv->value);
}

// ut64 API
// useful when the keys are numbers
SDB_API SdbHt* ht_new0_u();
SDB_API SdbHt* ht_new_u(PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize valueSize);
SDB_API void ht_foreach_u(SdbHt *ht, HtUForeachCallback cb, void *user);

static inline bool ht_insert_u(SdbHt* ht, ut64 key, void* value) {
	return ht_insert (ht, key, (ut64)(uintptr_t)value);
}

static inline bool ht_update_u(SdbHt* ht, ut64 key, void* value) {
	return ht_update (ht, key, (ut64)(uintptr_t)value);
}

static inline bool ht_delete_u(SdbHt* ht, ut64 key) {
	return ht_delete (ht, key);
}

static inline void* ht_find_u(SdbHt* ht, ut64 key, bool* found) {
	return (void *)(uintptr_t)ht_find (ht, key, found);
}

#endif // __HT_H
