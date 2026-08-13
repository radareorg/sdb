/* sdb - MIT - Copyright 2018-2026 - ret2libc, pancake, luc-tielen */

/* Common implementation for the cwisstable-backed maps (HtPU, HtSU, HtUU).
 * Included by ht_pu.c, ht_su.c and ht_uu.c after defining:
 *   HT          public handle type (e.g. HtPU)
 *   HT_(n)      public function name (e.g. ht_pu_##n)
 *   CW_(n)      cwisstable symbol (e.g. HtPU__##n)
 *   HT_KEY      public key parameter type (e.g. void *)
 *   HT_CB       foreach callback type (e.g. HtPUForEachCallback)
 *   HT_KEY_SAME(a, b) key equality for the update_key noop check
 * The cwisstable map copies entries on insert (dup'ing owned keys via the
 * declared copy policy), so keys passed in are never stored directly. */

SDB_API HT *HT_(new0)(void) {
	HT *hm = (HT *)sdb_gh_calloc (1, sizeof (HT));
	if (hm) {
		hm->inner = CW_(new) (0);
	}
	return hm;
}

SDB_API void HT_(free)(HT *hm) {
	if (hm) {
		CW_(destroy) (&hm->inner);
		sdb_gh_free (hm);
	}
}

SDB_API bool HT_(insert)(HT *hm, HT_KEY key, ut64 value) {
	assert (hm);
	CW_(Entry) entry = { .key = (CW_(Key))key, .val = value };
	return CW_(insert) (&hm->inner, &entry).inserted;
}

SDB_API bool HT_(update)(HT *hm, HT_KEY key, ut64 value) {
	assert (hm);
	CW_(Entry) entry = { .key = (CW_(Key))key, .val = value };
	CW_(Insert) result = CW_(insert) (&hm->inner, &entry);
	if (!result.inserted) {
		CW_(Entry) *existing = CW_(Iter_get) (&result.iter);
		existing->val = value;
	}
	return true;
}

// Update the key of an element in the hashtable
SDB_API bool HT_(update_key)(HT *hm, HT_KEY old_key, HT_KEY new_key) {
	assert (hm);
	CW_(Iter) iter = CW_(find) (&hm->inner, (const CW_(Key) *)&old_key);
	CW_(Entry) *entry = CW_(Iter_get) (&iter);
	if (!entry) {
		return false;
	}
	if (HT_KEY_SAME (old_key, new_key)) {
		return true;
	}
	CW_(Entry) new_entry = { .key = (CW_(Key))new_key, .val = entry->val };
	if (!CW_(insert) (&hm->inner, &new_entry).inserted) {
		return false;
	}
	// erase by key: the insert above can rehash and invalidate iterators
	return CW_(erase) (&hm->inner, (const CW_(Key) *)&old_key);
}

SDB_API bool HT_(delete)(HT *hm, HT_KEY key) {
	assert (hm);
	return CW_(erase) (&hm->inner, (const CW_(Key) *)&key);
}

SDB_API ut64 HT_(find)(HT *hm, HT_KEY key, bool *found) {
	assert (hm);
	if (found) {
		*found = false;
	}
	CW_(Iter) iter = CW_(find) (&hm->inner, (const CW_(Key) *)&key);
	CW_(Entry) *entry = CW_(Iter_get) (&iter);
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
SDB_API void HT_(foreach)(HT *hm, HT_CB cb, void *user) {
	assert (hm);
	CW_(CIter) iter;
	const CW_(Entry) *entry;
	for (iter = CW_(citer) (&hm->inner); (entry = CW_(CIter_get) (&iter)) != NULL; CW_(CIter_next) (&iter)) {
		if (!cb (user, entry->key, entry->val)) {
			return;
		}
	}
}
