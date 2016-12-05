/* radare2 - BSD 3 Clause License - crowell, pancake 2016 */

#include "ht.h"
#include "sdb.h"

/* tune the hashtable */
#define GROWABLE 0
#define USE_KEYLEN 1
#define EXCHANGE 1

// Sizes of the ht.
const int ht_primes_sizes[] = {
#if GROWABLE
	3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131,
	163, 197, 239, 293, 353, 431, 521, 631, 761, 919,
	1103, 1327, 1597, 1931, 2333, 2801, 3371, 4049, 4861,
	5839, 7013, 8419, 10103, 12143, 14591, 17519, 21023,
	25229, 30293, 36353, 43627, 52361, 62851, 75431, 90523,
	108631, 130363, 156437, 187751, 225307, 270371, 324449,
	389357, 467237, 560689, 672827, 807403, 968897, 1162687,
	1395263, 1674319, 2009191, 2411033, 2893249, 3471899,
	4166287, 4999559, 5999471, 7199369
#else
	1024,
#endif
};


// Create a new hashtable and return a pointer to it.
// size - number of buckets in the hashtable
// hashfunction - the function that does the hashing, must not be null.
// comparator - the function to check if values are equal, if NULL, just checks == (for storing ints).
// keydup - function to duplicate to key (eg strdup), if NULL just does =.
// valdup - same as keydup, but for values
// pair_free - function for freeing a keyvaluepair - if NULL just does free.
// calcsize - function to calculate the size of a value. if NULL, just stores 0.
static SdbHash* internal_ht_new(ut32 size, HashFunction hashfunction, ListComparator comparator, DupKey keydup, DupValue valdup, HtKvFreeFunc pair_free, CalcSize calcsize) {
	SdbHash* ht = calloc (1, sizeof (*ht));
	if (!ht) {
		return NULL;
	}
	ht->size = size;
	ht->count = 0;
	ht->prime_idx = 0;
	ht->load_factor = 1;
	ht->hashfn = hashfunction;
	ht->cmp = comparator? comparator: strcmp;
	ht->dupkey = keydup? keydup: strdup;
	ht->dupvalue = valdup? valdup: strdup;
	ht->table = calloc (ht->size, sizeof (SdbList*));
	ht->calcsize = calcsize? calcsize: strlen;
	ht->freefn = pair_free;
	ht->deleted = ls_newf (free);
	// Because we use calloc, each listptr will be NULL until used */
	return ht;
}

bool ht_delete_internal(SdbHash* ht, const char* key, ut32* hash) {
	SdbKv* kvp;
	SdbListIter* iter;
	ut32 computed_hash = hash ? *hash : ht->hashfn (key);
#if USE_KEYLEN
	ut32 key_len = ht->calcsize (key);
#endif
	ut32 bucket = computed_hash % ht->size;
	SdbList* list = ht->table[bucket];
	ls_foreach (list, iter, kvp) {
#if USE_KEYLEN
		if (key_len != kvp->key_len) {
			continue;
		}
#endif
		if (key == kvp->key || !ht->cmp (key, kvp->key)) {
#if EXCHANGE
			ls_split_iter (list, iter);
			ls_append (ht->deleted, iter);
			list->free (iter->data);
			iter->data = NULL;
#else
			ls_delete (list, iter);
#endif
			ht->count--;
			return true;
		}
	}
	return false;
}

SdbHash* ht_new() {
	return internal_ht_new (ht_primes_sizes[0], sdb_hash, strcmp, strdup,
			strdup, sdb_kv_free, strlen);
}

void ht_free(SdbHash* ht) {
	if (ht) {
		ut32 i;
		for (i = 0; i < ht->size; i++) {
			ls_free (ht->table[i]);
		}
		free (ht->table);
		ls_free (ht->deleted);
		free (ht);
	}
}

void ht_free_deleted(SdbHash* ht) {
	if (!ls_empty (ht->deleted)) {
		ls_free (ht->deleted);
		ht->deleted = ls_newf (free);
	}
}

// Increases the size of the hashtable by 2.
#if GROWABLE
static void internal_ht_grow(SdbHash* ht) {
	SdbHash* ht2;
	SdbHash swap;
	SdbKv* kvp;
	SdbListIter* iter;
	ut32 i;
	ut32 sz = ht_primes_sizes[ht->prime_idx];
	ht2 = internal_ht_new (sz, ht->hashfn, ht->cmp, ht->dupkey,
			ht->dupvalue, (HtKvFreeFunc)ht->freefn, ht->calcsize);
	ht2->prime_idx = ht->prime_idx;
	for (i = 0; i < ht->size; i++) {
		ls_foreach (ht->table[i], iter, kvp) {
			(void)ht_insert (ht2, kvp->key, kvp->value);
		}
	}
	// And now swap the internals.
	swap = *ht;
	*ht = *ht2;
	*ht2 = swap;
	ht_free (ht2);
}
#endif

// Inserts the key value pair key, value into the hashtable.
// if update is true, allow for updates, otherwise return false if the key
// already exists.
static bool internal_ht_insert(SdbHash* ht, bool update, const char* key, const char* value) {
	if (!ht || !key || !value) {
		return false;
	}
	SdbKv* kvp;
	ut32 hash = ht->hashfn (key);
	ut32 bucket;
	bool found = true;
	if (update) {
		(void)ht_delete_internal (ht, key, &hash);
	} else {
		(void)ht_find (ht, key, &found);
	}
	if (update || !found) {
		kvp = calloc (1, sizeof (SdbKv));
		if (kvp) {
			kvp->key = ht->dupkey (key);
			kvp->value = ht->dupvalue (value);
			kvp->key_len = ht->calcsize (kvp->key);
			bucket = hash % ht->size;
			kvp->expire = 0;
			kvp->value_len = ht->calcsize (kvp->value);
			if (!ht->table[bucket]) {
				ht->table[bucket] = ls_newf ((SdbListFree)ht->freefn);
			}
			ls_prepend (ht->table[bucket], kvp);
			ht->count++;
#if GROWABLE
			// Check if we need to grow the table.
			if (ht->count >= ht->load_factor * ht_primes_sizes[ht->prime_idx]) {
				if (ht->prime_idx < sizeof (ht_primes_sizes) / sizeof (ht_primes_sizes[0])) {
					ht->prime_idx++;
					internal_ht_grow (ht);
				}
			}
#endif
			return true;
		}
	}
	return false;
}

// Inserts the key value pair key, value into the hashtable.
// Doesn't allow for "update" of the value.
bool ht_insert(SdbHash* ht, const char* key, const char* value) {
	return internal_ht_insert (ht, false, key, value);
}

// Inserts the key value pair key, value into the hashtable.
// Does allow for "update" of the value.
bool ht_update(SdbHash* ht, const char* key, const char* value) {
	return internal_ht_insert (ht, true, key, value);
}

bool ht_insert_kvp(SdbHash* ht, SdbKv* kvp, bool update) {
	bool found;
	if (!ht || !kvp) {
		return false;
	}
	ut32 bucket, hash = ht->hashfn (kvp->key);
	if (update) {
		(void)ht_delete_internal (ht, kvp->key, &hash);
	} else {
		(void)ht_find (ht, kvp->key, &found);
	}
	if (update || !found) {
		bucket = hash % ht->size;
		if (!ht->table[bucket]) {
			ht->table[bucket] = ls_newf ((SdbListFree)ht->freefn);
		}
		ls_prepend (ht->table[bucket], kvp);
		ht->count++;
#if GROWABLE
		// Check if we need to grow the table.
		if (ht->count >= ht->load_factor * ht_primes_sizes[ht->prime_idx]) {
			ht->prime_idx++;
			internal_ht_grow (ht);
		}
#endif
		return true;
	}
	return false;
}

// Returns the corresponding SdbKv entry from the key.
// If `found` is not NULL, it will be set to true if the entry was found, false
// otherwise.
SdbKv* ht_find_kvp(SdbHash* ht, const char* key, bool* found) {
	ut32 hash;
	ut32 bucket;
	SdbListIter* iter;
	SdbKv* kvp;
	hash = ht->hashfn (key);
	bucket = hash % ht->size;
	ls_foreach (ht->table[bucket], iter, kvp) {
		bool match = !ht->cmp (key, kvp->key);
		if (match) {
			if (found) {
				*found = true;
			}
			return  kvp;
		}
	}
	if (found) {
		*found = false;
	}
	return NULL;
}

// Looks up the corresponding value from the key.
// If `found` is not NULL, it will be set to true if the entry was found, false
// otherwise.
char* ht_find(SdbHash* ht, const char* key, bool* found) {
	bool _found = false;
	if (!found) {
		found = &_found;
	}
	SdbKv* kvp = ht_find_kvp (ht, key, found);
	return (kvp && *found)? kvp->value : NULL;
}

// Deletes a kvp from the hash table from the key, if the pair exists.
bool ht_delete(SdbHash* ht, const char* key) {
	return ht_delete_internal (ht, key, NULL);
}
