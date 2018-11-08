#include "sdbht.h"

void sdbkv_fini(SdbKv *kv) {
	free (sdbkv_key (kv));
	free (sdbkv_value (kv));
}

SDB_API SdbHt* sdb_ht_new() {
	SdbHt *ht = ht_new_p ((PDupValue)strdup, (HtKvFreeFunc)sdbkv_fini, (PCalcSize)strlen);
	if (ht) {
		ht->opt.elem_size = sizeof (SdbKv);
	}
	return ht;
}

static bool sdb_ht_internal_insert(SdbHt* ht, const char* key,
				    const char* value, bool update) {
	if (!ht || !key || !value) {
		return false;
	}
	SdbKv kvp = {{ 0 }};
	sdbkv_set_key (&kvp, strdup (key));
	if (!sdbkv_key (&kvp)) {
		goto err;
	}
	sdbkv_set_value (&kvp, strdup (value));
	if (!sdbkv_value (&kvp)) {
		goto err;
	}
	kvp.base.key_len = strlen (sdbkv_key (&kvp));
	kvp.base.value_len = strlen (sdbkv_value (&kvp));
	kvp.expire = 0;
	return ht_insert_kv (ht, (HtKv*)&kvp, update);

 err:
	free (sdbkv_key (&kvp));
	free (sdbkv_value (&kvp));
	return false;
}

SDB_API bool sdb_ht_insert(SdbHt* ht, const char* key, const char* value) {
	return sdb_ht_internal_insert (ht, key, value, false);
}

SDB_API bool sdb_ht_insert_kvp(SdbHt* ht, SdbKv *kvp, bool update) {
	return ht_insert_kv (ht, (HtKv*)kvp, update);
}

SDB_API bool sdb_ht_update(SdbHt *ht, const char *key, const char*value) {
	return sdb_ht_internal_insert (ht, key, value, true);
}

SDB_API SdbKv* sdb_ht_find_kvp(SdbHt* ht, const char* key, bool* found) {
	return (SdbKv *)ht_find_kv (ht, (ut64)(uintptr_t)key, found);
}

SDB_API char* sdb_ht_find(SdbHt* ht, const char* key, bool* found) {
	return ht_find_p (ht, key, found);
}

SDB_API void sdb_ht_free(SdbHt *ht) {
	ht_free (ht);
}

SDB_API bool sdb_ht_delete(SdbHt* ht, const char *key) {
	return ht_delete_p (ht, key);
}
