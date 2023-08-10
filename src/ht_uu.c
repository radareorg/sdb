/* sdb - MIT - Copyright 2018-2023 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_uu.h"
#include "sdb/heap.h"
#include "sdb/cwisstable.h"

typedef uint64_t ut64;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtypedef-redefinition"
#endif
CWISS_DECLARE_FLAT_HASHMAP_DEFAULT(HtUU, ut64, ut64);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

SDB_API HtUU* ht_uu_new0(void) {
	HtUU *hm = sdb_gh_calloc (1, sizeof (HtUU));
	if (hm) {
		*hm = HtUU_new (0);
	}
	return hm;
}

SDB_API void ht_uu_free(HtUU *hm) {
	if (hm) {
		HtUU_destroy (hm);
		sdb_gh_free (hm);
	}
}

SDB_API bool ht_uu_insert(HtUU *hm, const ut64 key, ut64 value) {
	assert (hm);

	HtUU_Entry entry = { .key = key, .val = value };
	HtUU_Insert result = HtUU_insert (hm, &entry);
	return result.inserted;
}

SDB_API bool ht_uu_update(HtUU *hm, const ut64 key, ut64 value) {
	assert (hm);

	HtUU_Entry entry = { .key = key, .val = value };
	HtUU_Insert insert_result = HtUU_insert (hm, &entry);
	const bool should_update = !insert_result.inserted;
	if (should_update) {
		HtUU_Entry *existing_entry = HtUU_Iter_get (&insert_result.iter);
		existing_entry->val = value;
	}

	return true;
}

// Update the key of an element in the hashtable
SDB_API bool ht_uu_update_key(HtUU *hm, const ut64 old_key, const ut64 new_key) {
	assert (hm);

	HtUU_Iter iter = HtUU_find (hm, &old_key);
	HtUU_Entry *entry = HtUU_Iter_get (&iter);
	if (!entry) {
		return false;
	}

	// First try inserting the new key
	HtUU_Entry new_entry = { .key = new_key, .val = entry->val };
	HtUU_Insert result = HtUU_insert (hm, &new_entry);
	if (!result.inserted) {
		return false;
	}

	// Then remove entry for the old key
	HtUU_erase_at (iter);
	return true;
}

SDB_API bool ht_uu_delete(HtUU *hm, const ut64 key) {
	assert (hm);
	return HtUU_erase (hm, &key);
}

SDB_API ut64 ht_uu_find(HtUU *hm, const ut64 key, bool* found) {
	assert (hm);
	if (found) {
		*found = false;
	}

	HtUU_Iter iter = HtUU_find (hm, &key);
	HtUU_Entry *entry = HtUU_Iter_get (&iter);
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
SDB_API void ht_uu_foreach(HtUU *hm, HtUUForEachCallback cb, void *user) {
	assert (hm);
	HtUU_CIter iter;
	const HtUU_Entry *entry;
	for (iter = HtUU_citer (hm); (entry = HtUU_CIter_get (&iter)) != NULL; HtUU_CIter_next (&iter)) {
		if (!cb (user, entry->key, entry->val)) {
			return;
		}
	}
}
