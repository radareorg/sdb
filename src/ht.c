/* radare2 - BSD 3 Clause License - crowell, pancake 2016 */

#include "ht.h"
#include "sdb.h"

#define LOAD_FACTOR 0.9
#define S_ARRAY_SIZE(x) (sizeof (x) / sizeof (x[0]))

// Sizes of the ht.
static const int ht_primes_sizes[] = {
	3, 7, 11, 17, 23, 29, 37, 47, 59, 71, 89, 107, 131,
	163, 197, 239, 293, 353, 431, 521, 631, 761, 919,
	1103, 1327, 1597, 1931, 2333, 2801, 3371, 4049, 4861,
	5839, 7013, 8419, 10103, 12143, 14591, 17519, 21023,
	25229, 30293, 36353, 43627, 52361, 62851, 75431, 90523,
	108631, 130363, 156437, 187751, 225307, 270371, 324449,
	389357, 467237, 560689, 672827, 807403, 968897, 1162687,
	1395263, 1674319, 2009191, 2411033, 2893249, 3471899,
	4166287, 4999559, 5999471, 7199369
};

#define HASHFN(ht, k) ((ht)->hashfn ? (ht)->hashfn (k) : (ut32)(ut64)(k))
#define BUCKET(ht, k) ((HASHFN (ht, k) % (ht)->size))
#define DUPKEY(ht, k) ((ht)->dupkey ? (ht)->dupkey (k) : (void *)k)
#define DUPVAL(ht, v) ((ht)->dupvalue ? (ht)->dupvalue (v) : v)
#define CALCSIZEK(ht, k) ((ht)->calcsizeK ? (ht)->calcsizeK (k) : 0)
#define CALCSIZEV(ht, v) ((ht)->calcsizeV ? (ht)->calcsizeV (v) : 0)
#define FREEFN(ht, kv) do { if ((ht)->freefn) { (ht)->freefn (kv); } } while (0)
// when possible, use the precomputed prime numbers which help with collisions,
// otherwise, at least make the number odd with |1
#define COMPUTE_SIZE(idx, sz) ((idx) != UT32_MAX ? ht_primes_sizes[idx] : ((sz) | 1))

#define KV_AT(ht, bt, i) ((HtKv *)((char *)(bt)->arr + (i) * (ht)->elem_size))
#define NEXTKV(ht, kv) ((HtKv *)((char *)(kv) + (ht)->elem_size))

#define BUCKET_FOREACH(ht, bt, j, kv) \
	if ((bt)->arr) \
		for ((j) = 0, (kv) = (bt)->arr; j < (bt)->count; (j)++, (kv) = NEXTKV (ht, kv))

static inline bool is_kv_equal(SdbHt *ht, const char *key, const ut32 key_len, const HtKv *kv) {
	if (key_len != kv->key_len) {
		return false;
	}

	bool res = key == kv->key;
	if (!res && ht->cmp) {
		res = !ht->cmp (key, kv->key);
	}
	return res;
}

// Create a new hashtable and return a pointer to it.
// size - number of buckets in the hashtable
// hashfunction - the function that does the hashing, must not be null.
// comparator - the function to check if values are equal, if NULL, just checks
// == (for storing ints).
// keydup - function to duplicate to key (eg strdup), if NULL just does strup.
// valdup - same as keydup, but for values but if NULL just assign
// pair_free - function for freeing a keyvaluepair - if NULL just does free.
// calcsize - function to calculate the size of a value. if NULL, just stores 0.
static SdbHt* internal_ht_new(ut32 size, ut32 prime_idx, HashFunction hashfunction,
				ListComparator comparator, DupKey keydup,
				DupValue valdup, HtKvFreeFunc pair_free,
				CalcSize calcsizeK, CalcSize calcsizeV, size_t elem_size) {
	SdbHt* ht = calloc (1, sizeof (*ht));
	if (!ht) {
		return NULL;
	}
	ht->size = size;
	ht->count = 0;
	ht->prime_idx = prime_idx;
	ht->hashfn = hashfunction;
	ht->cmp = comparator;
	ht->dupkey = keydup;
	ht->dupvalue = valdup;
	ht->table = calloc (ht->size, sizeof (struct ht_bucket_t));
	if (!ht->table) {
		free (ht);
		return NULL;
	}
	ht->calcsizeK = calcsizeK;
	ht->calcsizeV = calcsizeV;
	ht->freefn = pair_free;
	ht->elem_size = elem_size;
	return ht;
}

SDB_API SdbHt* ht_new(DupValue valdup, HtKvFreeFunc pair_free, CalcSize calcsizeV) {
	return internal_ht_new (ht_primes_sizes[0], 0, (HashFunction)sdb_hash,
		(ListComparator)strcmp, (DupKey)strdup,
		valdup, pair_free, (CalcSize)strlen, calcsizeV, sizeof (HtKv));
}

SDB_API SdbHt* ht_new_size(ut32 initial_size, DupValue valdup, HtKvFreeFunc pair_free, CalcSize calcsizeV) {
	ut32 sz;
	int i = 0;

	while (i < S_ARRAY_SIZE (ht_primes_sizes) &&
		ht_primes_sizes[i] * LOAD_FACTOR < initial_size) {
		i++;
	}
	if (i == S_ARRAY_SIZE (ht_primes_sizes)) {
		i = UT32_MAX;
	}

	sz = COMPUTE_SIZE (i, (ut32)(initial_size * (2 - LOAD_FACTOR)));
	return internal_ht_new (sz, i, (HashFunction)sdb_hash,
		(ListComparator)strcmp, (DupKey)strdup,
		valdup, pair_free, (CalcSize)strlen, calcsizeV, sizeof (HtKv));
}

SDB_API void ht_free(SdbHt* ht) {
	if (!ht) {
		return;
	}

	ut32 i;
	for (i = 0; i < ht->size; i++) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		int j;

		if (ht->freefn) {
			BUCKET_FOREACH (ht, bt, j, kv) {
				ht->freefn (kv);
			}
		}

		free (bt->arr);
	}
	free (ht->table);
	free (ht);
}

// Increases the size of the hashtable by 2.
static void internal_ht_grow(SdbHt* ht) {
	SdbHt* ht2;
	SdbHt swap;
	ut32 i, idx, sz;

	idx = ht->prime_idx != UT32_MAX ? ht->prime_idx + 1 : UT32_MAX;
	if (idx == S_ARRAY_SIZE (ht_primes_sizes)) {
		idx = UT32_MAX;
	}
	sz = COMPUTE_SIZE (idx, ht->size * 2);

	ht2 = internal_ht_new (sz, idx, ht->hashfn, ht->cmp, ht->dupkey, ht->dupvalue,
		ht->freefn, ht->calcsizeK, ht->calcsizeV, ht->elem_size);

	for (i = 0; i < ht->size; i++) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		int j;

		BUCKET_FOREACH (ht, bt, j, kv) {
			ht_insert_kv (ht2, kv, false);
		}
	}
	// And now swap the internals.
	swap = *ht;
	*ht = *ht2;
	*ht2 = swap;

	ht2->freefn = NULL;
	ht_free (ht2);
}

bool ht_insert_kv(SdbHt *ht, HtKv *kv, bool update) {
	ut32 bucket = BUCKET (ht, kv->key);
	HtBucket *bt = &ht->table[bucket];
	HtKv *kvtmp;
	int j;

	BUCKET_FOREACH (ht, bt, j, kvtmp) {
		if (is_kv_equal (ht, kv->key, kv->key_len, kvtmp)) {
			if (update) {
				// XXX: just free value
				memcpy (kvtmp, kv, ht->elem_size);
				return true;
			}
			return false;
		}
	}

	HtKv *newkvarr = realloc (bt->arr, (bt->count + 1) * ht->elem_size);
	if (!newkvarr) {
		return false;
	}

	bt->arr = newkvarr;
	memcpy (KV_AT (ht, bt, bt->count), kv, ht->elem_size);
	bt->count++;
	ht->count++;
	return true;
}

// Inserts the key value pair key, value into the hashtable.
// Doesn't allow for "update" of the value.
SDB_API bool ht_insert(SdbHt* ht, const char* key, void* value) {
	ut32 bucket = BUCKET (ht, key);
	ut32 key_len = CALCSIZEK (ht, key);
	HtBucket *bt = &ht->table[bucket];
	HtKv *kv;
	int j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			return false;
		}
	}

	HtKv *newkvarr = realloc (bt->arr, (bt->count + 1) * ht->elem_size);
	if (!newkvarr) {
		return false;
	}

	bt->arr = newkvarr;
	kv = KV_AT (ht, bt, bt->count);
	kv->key = DUPKEY (ht, key);
	kv->key_len = key_len;
	kv->value = DUPVAL (ht, value);
	kv->value_len = CALCSIZEV (ht, value);
	bt->count++;
	ht->count++;

	if (ht->count >= LOAD_FACTOR * ht->size) {
		internal_ht_grow (ht);
	}
	return true;
}

// Inserts the key value pair key, value into the hashtable.
// Does allow for "update" of the value.
SDB_API bool ht_update(SdbHt* ht, const char* key, void* value) {
	ut32 bucket = BUCKET (ht, key);
	ut32 key_len = CALCSIZEK (ht, key);
	HtBucket *bt = &ht->table[bucket];
	HtKv *kv;
	int j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			// XXX: just free value
			kv->value = DUPVAL (ht, value);
			kv->value_len = CALCSIZEV (ht, value);
			return true;
		}
	}
	return false;
}

// Returns the corresponding SdbKv entry from the key.
// If `found` is not NULL, it will be set to true if the entry was found, false
// otherwise.
HtKv* ht_find_kv(SdbHt* ht, const char* key, bool* found) {
	ut32 bucket = BUCKET (ht, key);

	if (found) {
		*found = false;
	}

	HtBucket *bt = &ht->table[bucket];
	ut32 key_len = CALCSIZEK (ht, key);
	HtKv *kv;
	int j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			if (found) {
				*found = true;
			}
			return kv;
		}
	}
	return NULL;
}

// Looks up the corresponding value from the key.
// If `found` is not NULL, it will be set to true if the entry was found, false
// otherwise.
SDB_API void* ht_find(SdbHt* ht, const char* key, bool* found) {
	ut32 bucket = BUCKET (ht, key);

	if (found) {
		*found = false;
	}

	HtBucket *bt = &ht->table[bucket];
	ut32 key_len = CALCSIZEK (ht, key);
	HtKv *kv;
	int j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			if (found) {
				*found = true;
			}
			return kv->value;
		}
	}
	return NULL;
}

// Deletes a entry from the hash table from the key, if the pair exists.
SDB_API bool ht_delete(SdbHt* ht, const char* key) {
	ut32 bucket = BUCKET (ht, key);
	HtBucket *bt = &ht->table[bucket];
	ut32 key_len = CALCSIZEK (ht, key);
	HtKv *kv;
	int j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			FREEFN (ht, kv);
			void *src = NEXTKV (ht, kv);
			memmove (kv, src, (bt->count - j - 1) * ht->elem_size);
			bt->count--;
			ht->count--;
			return true;
		}
	}
	return false;
}

SDB_API void ht_foreach(SdbHt *ht, HtForeachCallback cb, void *user) {
	int i;

	for (i = 0; i < ht->size; ++i) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		int j;

		BUCKET_FOREACH (ht, bt, j, kv) {
			if (!cb (user, kv->key, kv->value)) {
				return;
			}
		}
	}
}
