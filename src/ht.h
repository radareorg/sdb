/* radare2 - BSD 3 Clause License - 2016 - crowell */

#ifndef __HT_H
#define __HT_H

#include "ls.h"
#include "types.h"

/** keyvalue pair **/
typedef struct sdb_kv {
	char *key;
	char *value;
	ut32 value_len;
	ut32 cas;
	ut64 expire;
} SdbKv;

typedef void (*HtKvFreeFunc)(SdbKv*);
typedef char* (*DupKey)(const char*);
typedef char* (*DupValue)(const char*);
typedef size_t (*CalcSize)(const char*);
typedef ut32 (*HashFunction)(const char*);
typedef int (*ListComparator)(const char *a, const char *b);

SdbKv* sdb_kv_new(const char *k, const char *v);

/** ht **/
typedef struct ht_t {
	ut32 size; // size of the hash table in buckets.
	ut32 count; // number of stored elements.
	ListComparator cmp; // Function for comparing values. Returns 0 if eq.
	HashFunction hashfn; // Function for hashing items in the hash table.
	DupKey dupkey; // Function for making a copy of key to store in the entry. NULL for just copying the pointer.
	DupValue dupvalue; // Function for making a copy of value to store in the entry. NULL for just copying the pointer.
	CalcSize calcsize; // Function to determine the size of an object. NULL will just set size to 0 for the kvp (usually not needed).
	HtKvFreeFunc freefn; // Function to free the keyvalue store, if NULL, just calls regular free.
	SdbList/*<SdbKv>*/** table;  // Actual table.
	ut32 load_factor; // load factor before doubling in size.
	ut32 prime_idx;
} SdbHash;

// Create a new RHashTable2.
SdbHash* ht_new();

// Destroy a hashtable and all of its entries.
void ht_free(SdbHash* ht);

// Insert a new Key-Value pair into the hashtable. If the key already exists, returns false.
bool ht_insert(SdbHash* ht, const char* key, const char* value);

// Insert a new Key-Value pair into the hashtable, or updates the value if the key already exists.
bool ht_update(SdbHash* ht, const char* key, const char* value);

// Delete a key from the hashtable.
bool ht_delete(SdbHash* ht, const char* key);

// Find the value corresponding to the matching key.
char* ht_find(SdbHash* ht, const char* key, bool* found);

// Find the KeyValuePair corresponding to the matching key.
SdbKv* ht_find_kvp(SdbHash* ht, const char* key, bool* found);

// Insert an existing keyvalue pair to the hashtable.
bool ht_insert_kvp(SdbHash* ht, SdbKv* kvp, bool update);

#endif // __HT_H
