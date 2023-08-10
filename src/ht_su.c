/* sdb - MIT - Copyright 2018-2023 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_su.h"
#include "sdb/heap.h"
#include "sdb/sdb.h"
#include "../../../libr/include/cwisstable.h"  // XXX use cwisstable from sdb itself

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtypedef-redefinition"
#endif

static inline void string_copy(void *dst, const void *src);
static inline void string_dtor(void *val);
static inline size_t string_hash(const void *val);
static inline bool string_eq(const void *a, const void *b);

CWISS_DECLARE_FLAT_HASHMAP(HtSU, char*, float, string_copy, string_dtor, string_hash, string_eq);

static inline void string_copy(void *dst_, const void *src_) {
  const HtSU_Entry *src = src_;
  HtSU_Entry *dst = dst_;

  const size_t len = strlen (src->key);
  dst->key = sdb_gh_malloc (len + 1);
  dst->val = src->val;
  memcpy (dst->key, src->key, len + 1);
}

static inline void string_dtor(void *val) {
  char *str = *(char**)val;
  sdb_gh_free (str);
}

static inline size_t string_hash(const void *val) {
  const char *str = *(const char *const *)val;
  const size_t len = strlen (str);
  CWISS_FxHash_State state = 0;
  CWISS_FxHash_Write (&state, str, len);
  return state;
}

static inline bool string_eq(const void *a, const void *b) {
  const char *ap = *(const char* const *)a;
  const char *bp = *(const char* const *)b;
  return strcmp (ap, bp) == 0;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

SDB_API HtSU* ht_su_new0(void) {
	HtSU *hm = sdb_gh_calloc (1, sizeof (HtSU));
	if (hm) {
		*hm = HtSU_new (0);
	}
	return hm;
}

SDB_API void ht_su_free(HtSU *hm) {
	if (hm) {
		HtSU_destroy (hm);
		sdb_gh_free (hm);
	}
}

SDB_API bool ht_su_insert(HtSU *hm, const char *key, ut64 value) {
	assert (hm && key);

	char *key_copy = sdb_strdup (key);
	if (!key_copy) {
		return false;
	}

	HtSU_Entry entry = { .key = key_copy, .val = value };
	HtSU_Insert result = HtSU_insert (hm, &entry);
	if (!result.inserted) {
		sdb_gh_free (key_copy);
		return false;
	}
	return true;
}

SDB_API bool ht_su_update(HtSU *hm, const char *key, ut64 value) {
	assert (hm && key);

	char *key_copy = sdb_strdup (key);
	if (!key_copy) {
		return false;
	}

	HtSU_Entry entry = { .key = key_copy, .val = value };
	HtSU_Insert insert_result = HtSU_insert (hm, &entry);
	if (!insert_result.inserted) {
		sdb_gh_free (key_copy);

		HtSU_Entry *existing_entry = HtSU_Iter_get (&insert_result.iter);
		existing_entry->val = value;
	}

	return true;
}

// Update the key of an element in the hashtable
SDB_API bool ht_su_update_key(HtSU *hm, const char *old_key, const char *new_key) {
	assert (hm && old_key && new_key);

	HtSU_Iter iter = HtSU_find (hm, (const HtSU_Key*) &old_key);
	HtSU_Entry *entry = HtSU_Iter_get (&iter);
	if (!entry) {
		return false;
	}

	// Do nothing if keys are the same
	if (SDB_UNLIKELY (strcmp (old_key, new_key) == 0)) {
		return true;
	}

	char *key_copy = sdb_strdup (new_key);
	if (!key_copy) {
		return false;
	}

	// First try inserting the new key
	HtSU_Entry new_entry = { .key = key_copy, .val = entry->val };
	HtSU_Insert result = HtSU_insert (hm, &new_entry);
	if (!result.inserted) {
		sdb_gh_free (key_copy);
		return false;
	}

	// Then remove entry for the old key
	HtSU_erase_at (iter);
	return true;
}

SDB_API bool ht_su_delete(HtSU *hm, const char *key) {
	assert (hm && key);
	return HtSU_erase (hm, (const HtSU_Key*) &key);
}

SDB_API ut64 ht_su_find(HtSU *hm, const char *key, bool* found) {
	assert (hm && key);

	if (found) {
		*found = false;
	}

	HtSU_Iter iter = HtSU_find (hm, (const HtSU_Key*) &key);
	HtSU_Entry *entry = HtSU_Iter_get (&iter);
	if (!entry) {
		return 0;
	}

	if (found) {
		*found = true;
	}
	return entry->val;
}

// Iterates over all elements in the hashtable, calling the cb function on each Kv.
// If the cb returns false, the iteration is stopped.
// cb should not modify the hashtable.
SDB_API void ht_su_foreach(HtSU *hm, HtSUForEachCallback cb, void *user) {
	assert (hm);
	HtSU_CIter iter;
	const HtSU_Entry *entry;

	for (iter = HtSU_citer (hm); (entry = HtSU_CIter_get (&iter)) != NULL; HtSU_CIter_next (&iter)) {
		if (!cb (user, entry->key, entry->val)) {
			return;
		}
	}
}
