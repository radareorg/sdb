/* radare2 - BSD 3 Clause License - crowell, pancake, ret2libc 2016-2018 */

#include "ht.h"
#include "sdb.h"

#define LOAD_FACTOR 1
#define S_ARRAY_SIZE(x) (sizeof (x) / sizeof (x[0]))

struct p_user {
	void *user;
	HtPForeachCallback cb;
};

struct u_user {
	void *user;
	HtUForeachCallback cb;
};


// Sizes of the ht.
static const ut32 ht_primes_sizes[] = {
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

static inline ut32 hashfn(SdbHt *ht, ut64 k) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.hashfn ? ht->opt.sub.p.hashfn (ht_u2ptr (k)) : (ut32)(size_t)(k);
	case HT_TYPE_U:
		return ht->opt.sub.u.hashfn ? ht->opt.sub.u.hashfn (k) : (ut32)(size_t)(k);
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.hashfn ? ht->opt.sub.n.hashfn (k) : (ut32)(size_t)(k);
	}
}

static inline ut32 bucketfn(SdbHt *ht, ut64 k) {
	return hashfn (ht, k) % ht->size;
}

static inline ut64 dupkey(SdbHt *ht, ut64 k) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.dupkey ? ht_ptr2u (ht->opt.sub.p.dupkey (ht_u2ptr (k))) : k;
	case HT_TYPE_U:
		return ht->opt.sub.u.dupkey ? ht->opt.sub.u.dupkey (k) : k;
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.dupkey ? ht->opt.sub.n.dupkey (k) : k;
	}
}

static inline ut64 dupval(SdbHt *ht, ut64 v) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.dupvalue ? ht_ptr2u (ht->opt.sub.p.dupvalue (ht_u2ptr (v))) : v;
	case HT_TYPE_U:
		return ht->opt.sub.u.dupvalue ? ht_ptr2u (ht->opt.sub.u.dupvalue (ht_u2ptr (v))) : v;
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.dupvalue ? ht->opt.sub.n.dupvalue (v) : v;
	}
}

static inline ut32 calcsize_key(SdbHt *ht, ut64 k) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.calcsizeK ? ht->opt.sub.p.calcsizeK (ht_u2ptr (k)) : 0;
	case HT_TYPE_U:
		return ht->opt.sub.u.calcsizeK ? ht->opt.sub.u.calcsizeK (k) : 0;
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.calcsizeK ? ht->opt.sub.n.calcsizeK (k) : 0;
	}
}

static inline ut32 calcsize_val(SdbHt *ht, ut64 v) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.calcsizeV ? ht->opt.sub.p.calcsizeV (ht_u2ptr (v)) : 0;
	case HT_TYPE_U:
		return ht->opt.sub.u.calcsizeV ? ht->opt.sub.u.calcsizeV (ht_u2ptr (v)) : 0;
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.calcsizeV ? ht->opt.sub.n.calcsizeV (v) : 0;
	}
}

static inline bool cmpfn(SdbHt *ht, ut64 key1, ut64 key2) {
	switch (ht->opt.type) {
	case HT_TYPE_P:
		return ht->opt.sub.p.cmp ? !ht->opt.sub.p.cmp (ht_u2ptr (key1), ht_u2ptr (key2)) : false;
	case HT_TYPE_U:
		return ht->opt.sub.u.cmp ? !ht->opt.sub.u.cmp (key1, key2) : false;
	case HT_TYPE_NONE:
	default:
		return ht->opt.sub.n.cmp ? !ht->opt.sub.n.cmp (key1, key2) : false;
	}
}

static inline void freefn(SdbHt *ht, HtKv *kv) {
	if (ht->opt.freefn) {
		ht->opt.freefn (kv);
	}
}

static inline ut32 compute_size(ut32 idx, ut32 sz) {
	// when possible, use the precomputed prime numbers which help with
	// collisions, otherwise, at least make the number odd with |1
	return idx != UT32_MAX ? ht_primes_sizes[idx] : (sz | 1);
}

static inline bool is_kv_equal(SdbHt *ht, ut64 key, const ut32 key_len, const HtKv *kv) {
	if (key_len != kv->key_len) {
		return false;
	}

	bool res = key == kv->key;
	if (!res) {
		res = cmpfn (ht, key, kv->key);
	}
	return res;
}

static inline HtKv *kv_at(SdbHt *ht, HtBucket *bt, ut32 i) {
	return (HtKv *)((char *)bt->arr + i * ht->opt.elem_size);
}

static inline HtKv *next_kv(SdbHt *ht, HtKv *kv) {
	return (HtKv *)((char *)kv + ht->opt.elem_size);
}

#define BUCKET_FOREACH(ht, bt, j, kv)					\
	if ((bt)->arr)							\
		for ((j) = 0, (kv) = (bt)->arr; (j) < (bt)->count; (j)++, (kv) = next_kv (ht, kv))

#define BUCKET_FOREACH_SAFE(ht, bt, j, count, kv)			\
	if ((bt)->arr)							\
		for ((j) = 0, (kv) = (bt)->arr, (count) = (ht)->count;	\
		     (j) < (bt)->count;					\
		     (j) = (count) == (ht)->count? j + 1: j, (kv) = (count) == (ht)->count? next_kv (ht, kv): kv, (count) = (ht)->count)

SDB_API HtPOptions *ht_p_options_init(HtOptions *opt, HtKvFreeFunc freefn) {
	memset (opt, 0, sizeof (*opt));
	opt->elem_size = sizeof (HtKv);
	opt->type = HT_TYPE_P;
	opt->freefn = freefn;
	return &opt->sub.p;
}

SDB_API HtUOptions *ht_u_options_init(HtOptions *opt, HtKvFreeFunc freefn) {
	memset (opt, 0, sizeof (*opt));
	opt->elem_size = sizeof (HtKv);
	opt->type = HT_TYPE_U;
	opt->freefn = freefn;
	return &opt->sub.u;
}

SDB_API HtNOptions *ht_n_options_init(HtOptions *opt, HtKvFreeFunc freefn) {
	memset (opt, 0, sizeof (*opt));
	opt->elem_size = sizeof (HtKv);
	opt->type = HT_TYPE_NONE;
	opt->freefn = freefn;
	return &opt->sub.n;
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
static SdbHt* internal_ht_new(ut32 size, ut32 prime_idx, HtOptions *opt) {
	SdbHt* ht = calloc (1, sizeof (*ht));
	if (!ht) {
		return NULL;
	}
	ht->size = size;
	ht->count = 0;
	ht->prime_idx = prime_idx;
	ht->table = calloc (ht->size, sizeof (struct ht_bucket_t));
	if (!ht->table) {
		free (ht);
		return NULL;
	}
	ht->opt = *opt;
	// if type is not provided, assume we are dealing with a NONE SdbHt
	if (ht->opt.type == 0) {
		ht->opt.type = HT_TYPE_NONE;
	}

	// if elem_size is not provided, assume we are dealing with a regular
	// SdbHt, with HtKv as elements
	if (ht->opt.elem_size == 0) {
		ht->opt.elem_size = sizeof (HtKv);
	}
	return ht;
}

static void ht_string_options_init(HtOptions *opt, HtKvFreeFunc freefn, PDupValue valdup, PCalcSize calcsizeV) {
	HtPOptions *popt = ht_p_options_init (opt, freefn);
	popt->cmp = (PListComparator)strcmp;
	popt->hashfn = (PHashFunction)sdb_hash;
	popt->dupkey = (PDupKey)strdup;
	popt->dupvalue = valdup;
	popt->calcsizeK = (PCalcSize)strlen;
	popt->calcsizeV = calcsizeV;
}

static SdbHt* internal_ht_default_new(ut32 size, ut32 prime_idx, PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize calcsizeV) {
	HtOptions opt;
	ht_string_options_init (&opt, pair_free, valdup, calcsizeV);
	return internal_ht_new (size, prime_idx, &opt);
}

SDB_API SdbHt* ht_new_p(PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize calcsizeV) {
	return internal_ht_default_new (ht_primes_sizes[0], 0, valdup, pair_free, calcsizeV);
}

SDB_API SdbHt* ht_new_u(PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize calcsizeV) {
	HtOptions opt;
	HtUOptions *uopt = ht_u_options_init (&opt, pair_free);
	uopt->dupvalue = valdup;
	uopt->calcsizeV = calcsizeV;
	return internal_ht_new (ht_primes_sizes[0], 0, &opt);
}

SDB_API SdbHt* ht_new0_u() {
	return ht_new_u (NULL, NULL, NULL);
}

static void free_kv_key(HtKv *kv) {
	free ((void *)(uintptr_t)kv->key);
}

SDB_API SdbHt* ht_new0_p() {
	return ht_new_p (NULL, free_kv_key, NULL);
}

SDB_API SdbHt* ht_new_p_size(ut32 initial_size, PDupValue valdup, HtKvFreeFunc pair_free, PCalcSize calcsizeV) {
	ut32 i = 0;

	while (i < S_ARRAY_SIZE (ht_primes_sizes) &&
		ht_primes_sizes[i] * LOAD_FACTOR < initial_size) {
		i++;
	}
	if (i == S_ARRAY_SIZE (ht_primes_sizes)) {
		i = UT32_MAX;
	}

	ut32 sz = compute_size (i, (ut32)(initial_size * (2 - LOAD_FACTOR)));
	return internal_ht_default_new (sz, i, valdup, pair_free, calcsizeV);
}

SDB_API SdbHt *ht_new_opt(HtOptions *opt) {
	return internal_ht_new (ht_primes_sizes[0], 0, opt);
}

SDB_API void ht_free(SdbHt* ht) {
	if (!ht) {
		return;
	}

	ut32 i;
	for (i = 0; i < ht->size; i++) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		ut32 j;

		if (ht->opt.freefn) {
			BUCKET_FOREACH (ht, bt, j, kv) {
				ht->opt.freefn (kv);
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
	ut32 idx = ht->prime_idx != UT32_MAX ? ht->prime_idx + 1 : UT32_MAX;
	ut32 sz = compute_size (idx, ht->size * 2);
	ut32 i;

	ht2 = internal_ht_new (sz, idx, &ht->opt);

	for (i = 0; i < ht->size; i++) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		ut32 j;

		BUCKET_FOREACH (ht, bt, j, kv) {
			ht_insert_kv (ht2, kv, false);
		}
	}
	// And now swap the internals.
	swap = *ht;
	*ht = *ht2;
	*ht2 = swap;

	ht2->opt.freefn = NULL;
	ht_free (ht2);
}

static void check_growing(SdbHt *ht) {
	if (ht->count >= LOAD_FACTOR * ht->size) {
		internal_ht_grow (ht);
	}
}

static HtKv *reserve_kv(SdbHt *ht, ut64 key, const int key_len, bool update) {
	HtBucket *bt = &ht->table[bucketfn (ht, key)];
	HtKv *kvtmp;
	ut32 j;

	BUCKET_FOREACH (ht, bt, j, kvtmp) {
		if (is_kv_equal (ht, key, key_len, kvtmp)) {
			if (update) {
				freefn (ht, kvtmp);
				return kvtmp;
			}
			return NULL;
		}
	}

	HtKv *newkvarr = realloc (bt->arr, (bt->count + 1) * ht->opt.elem_size);
	if (!newkvarr) {
		return NULL;
	}

	bt->arr = newkvarr;
	bt->count++;
	ht->count++;
	return kv_at (ht, bt, bt->count - 1);
}

SDB_API bool ht_insert_kv(SdbHt *ht, HtKv *kv, bool update) {
	HtKv *kv_dst = reserve_kv (ht, kv->key, kv->key_len, update);
	if (!kv_dst) {
		return false;
	}

	memcpy (kv_dst, kv, ht->opt.elem_size);
	check_growing (ht);
	return true;
}

static bool insert_update(SdbHt *ht, ut64 key, ut64 value, bool update) {
	ut32 key_len = calcsize_key (ht, key);
	HtKv* kv_dst = reserve_kv (ht, key, key_len, update);
	if (!kv_dst) {
		return false;
	}

	kv_dst->key = dupkey (ht, key);
	kv_dst->key_len = key_len;
	kv_dst->value = dupval (ht, value);
	kv_dst->value_len = calcsize_val (ht, value);
	check_growing (ht);
	return true;
}

// Inserts the key value pair key, value into the hashtable.
// Doesn't allow for "update" of the value.
SDB_API bool ht_insert(SdbHt* ht, ut64 key, ut64 value) {
	return insert_update (ht, key, value, false);
}

// Inserts the key value pair key, value into the hashtable.
// Does allow for "update" of the value.
SDB_API bool ht_update(SdbHt* ht, ut64 key, ut64 value) {
	return insert_update (ht, key, value, true);
}

// Returns the corresponding SdbKv entry from the key.
// If `found` is not NULL, it will be set to true if the entry was found, false
// otherwise.
SDB_API HtKv* ht_find_kv(SdbHt* ht, ut64 key, bool *found) {
	if (found) {
		*found = false;
	}

	HtBucket *bt = &ht->table[bucketfn (ht, key)];
	ut32 key_len = calcsize_key (ht, key);
	HtKv *kv;
	ut32 j;

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
SDB_API ut64 ht_find(SdbHt* ht, ut64 key, bool* found) {
	HtKv *res = ht_find_kv (ht, key, found);
	return res ? res->value : 0;
}

// Deletes a entry from the hash table from the key, if the pair exists.
SDB_API bool ht_delete(SdbHt* ht, ut64 key) {
	HtBucket *bt = &ht->table[bucketfn (ht, key)];
	ut32 key_len = calcsize_key (ht, key);
	HtKv *kv;
	ut32 j;

	BUCKET_FOREACH (ht, bt, j, kv) {
		if (is_kv_equal (ht, key, key_len, kv)) {
			freefn (ht, kv);
			void *src = next_kv (ht, kv);
			memmove (kv, src, (bt->count - j - 1) * ht->opt.elem_size);
			bt->count--;
			ht->count--;
			return true;
		}
	}
	return false;
}

SDB_API void ht_foreach(SdbHt *ht, HtForeachCallback cb, void *user) {
	ut32 i;

	for (i = 0; i < ht->size; ++i) {
		HtBucket *bt = &ht->table[i];
		HtKv *kv;
		ut32 j, count;

		BUCKET_FOREACH_SAFE (ht, bt, j, count, kv) {
			if (!cb (user, kv->key, kv->value)) {
				return;
			}
		}
	}
}

bool p_foreach_callback(void *user, ut64 key, ut64 value) {
	struct p_user *pu = user;
	pu->cb (pu->user, (const void *)(uintptr_t)key, (void *)(uintptr_t)value);
	return 1;
}

SDB_API void ht_foreach_p(SdbHt *ht, HtPForeachCallback cb, void *user) {
	struct p_user pu = {.user = user, .cb = cb};
	ht_foreach (ht, p_foreach_callback, &pu);
}

bool u_foreach_callback(void *user, ut64 key, ut64 value) {
	struct u_user *uu = user;
	uu->cb (uu->user, key, (void *)(uintptr_t)value);
	return 1;
}

SDB_API void ht_foreach_u(SdbHt *ht, HtUForeachCallback cb, void *user) {
	struct u_user uu = {.user = user, .cb = cb};
	ht_foreach (ht, u_foreach_callback, &uu);
}
