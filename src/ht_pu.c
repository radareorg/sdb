/* sdb - MIT - Copyright 2018-2023 - ret2libc, pancake, luc-tielen */

#include <stdint.h>
#include "sdb/ht_pu.h"
#include "sdb/heap.h"
#include "sdb/cwisstable.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtypedef-redefinition"
#endif
CWISS_DECLARE_FLAT_HASHMAP_DEFAULT(HtPU, void*, ut64);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

SDB_API HtPU* ht_pu_new0(void) {
	HtPU *hm = sdb_gh_calloc (1, sizeof (HtPU));
	if (hm) {
		*hm = HtPU_new (0);
	}
	return hm;
}

SDB_API void ht_pu_free(HtPU *hm) {
	if (hm) {
		HtPU_destroy (hm);
		sdb_gh_free (hm);
	}
}

SDB_API bool ht_pu_insert(HtPU *hm, void *key, ut64 value) {
	assert (hm);

	HtPU_Entry entry = { .key = key, .val = value };
	HtPU_Insert result = HtPU_insert (hm, &entry);
	return result.inserted;
}

SDB_API bool ht_pu_update(HtPU *hm, void *key, ut64 value) {
	assert (hm);

	HtPU_Entry entry = { .key = key, .val = value };
	HtPU_Insert insert_result = HtPU_insert (hm, &entry);
	const bool should_update = !insert_result.inserted;
	if (should_update) {
		HtPU_Entry *existing_entry = HtPU_Iter_get (&insert_result.iter);
		existing_entry->val = value;
	}

	return true;
}

// Update the key of an element in the hashtable
SDB_API bool ht_pu_update_key(HtPU *hm, void *old_key, void *new_key) {
	assert (hm);

	HtPU_Iter iter = HtPU_find (hm, &old_key);
	HtPU_Entry *entry = HtPU_Iter_get (&iter);
	if (!entry) {
		return false;
	}

	// First try inserting the new key
	HtPU_Entry new_entry = { .key = new_key, .val = entry->val };
	HtPU_Insert result = HtPU_insert (hm, &new_entry);
	if (!result.inserted) {
		return false;
	}

	// Then remove entry for the old key
	HtPU_erase_at (iter);
	return true;
}

SDB_API bool ht_pu_delete(HtPU *hm, void *key) {
	assert (hm);
	return HtPU_erase (hm, &key);
}

SDB_API ut64 ht_pu_find(HtPU *hm, void *key, bool* found) {
	assert (hm);
	if (found) {
		*found = false;
	}

	HtPU_Iter iter = HtPU_find (hm, &key);
	HtPU_Entry *entry = HtPU_Iter_get (&iter);
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
SDB_API void ht_pu_foreach(HtPU *hm, HtPUForEachCallback cb, void *user) {
	assert (hm);
	HtPU_CIter iter;
	const HtPU_Entry *entry;
	for (iter = HtPU_citer (hm); (entry = HtPU_CIter_get (&iter)) != NULL; HtPU_CIter_next (&iter)) {
		if (!cb (user, entry->key, entry->val)) {
			return;
		}
	}
}
