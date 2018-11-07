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
typedef size_t (*CalcSize)(ut64);
typedef ut32 (*HashFunction)(ut64);
typedef int (*ListComparator)(ut64, ut64);
typedef bool (*HtForeachCallback)(void *user, ut64, ut64);
typedef bool (*HtPForeachCallback)(void *user, const void *, void *);
typedef bool (*HtUForeachCallback)(void *user, ut64, void *);

typedef struct ht_bucket_t {
	HtKv *arr;
	ut32 count;
} HtBucket;

typedef struct ht_options_t {
	ListComparator cmp;   	// Function for comparing values. Returns 0 if eq.
	HashFunction hashfn;  	// Function for hashing items in the hash table.
	DupKey dupkey;  	// Function for making a copy of key
	DupValue dupvalue;  	// Function for making a copy of value
	CalcSize calcsizeK;     // Function to determine the key's size
	CalcSize calcsizeV;  	// Function to determine the value's size
	HtKvFreeFunc freefn;  	// Function to free the keyvalue store
	size_t elem_size;       // Size of each HtKv element (useful for subclassing like SdbKv)
} HtOptions;

/** ht **/
typedef struct ht_t {
	ut32 size;	  // size of the hash table in buckets.
	ut32 count;	  // number of stored elements.
	HtBucket* table;  // Actual table.
	ut32 prime_idx;
	HtOptions opt;
} SdbHt;

// Create a new RHashTable.
// If hashfunction is NULL it will be used sdb_hash internally
// If keydup or valdup are null it will be used an assignment
// If keySize or valueSize are null it will be used strlen internally
SDB_API SdbHt* ht_new0();
SDB_API SdbHt* ht_new(DupValue valdup, HtKvFreeFunc pair_free, CalcSize valueSize);
SDB_API SdbHt* ht_new_size(ut32 initial_size, DupValue valdup, HtKvFreeFunc pair_free, CalcSize valueSize);
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


// pointer API
// Useful when the keys are pointers
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

SDB_API void ht_foreach_p(SdbHt *ht, HtPForeachCallback cb, void *user);

// ut64 API
// useful when the keys are numbers
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

SDB_API void ht_foreach_u(SdbHt *ht, HtUForeachCallback cb, void *user);

#endif // __HT_H
