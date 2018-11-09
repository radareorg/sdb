#include "sdbht.h"

void sdbkv_fini(SdbKv *kv) {
	free (kv->base.key);
	free (kv->base.value);
}

SDB_API SdbPHt* sdb_ht_new() {
	SdbPHt *ht = ht_p_new ((HtPDupValue)strdup, (HtPKvFreeFunc)sdbkv_fini, (HtPCalcSizeV)strlen);
	if (ht) {
		ht->opt.elem_size = sizeof (SdbKv);
	}
	return ht;
}

static bool sdb_ht_internal_insert(SdbPHt* ht, const char* key,
				    const char* value, bool update) {
	if (!ht || !key || !value) {
		return false;
	}
	SdbKv kvp = {{ 0 }};
	kvp.base.key = strdup ((void *)key);
	if (!kvp.base.key) {
		goto err;
	}
	kvp.base.value = strdup ((void *)value);
	if (!kvp.base.value) {
		goto err;
	}
	kvp.base.key_len = strlen (kvp.base.key);
	kvp.base.value_len = strlen (kvp.base.value);
	kvp.expire = 0;
	return ht_p_insert_kv (ht, (HtPKv*)&kvp, update);

 err:
	free (kvp.base.key);
	free (kvp.base.value);
	return false;
}

SDB_API bool sdb_ht_insert(SdbPHt* ht, const char* key, const char* value) {
	return sdb_ht_internal_insert (ht, key, value, false);
}

SDB_API bool sdb_ht_insert_kvp(SdbPHt* ht, SdbKv *kvp, bool update) {
	return ht_p_insert_kv (ht, (HtPKv*)kvp, update);
}

SDB_API bool sdb_ht_update(SdbPHt *ht, const char *key, const char*value) {
	return sdb_ht_internal_insert (ht, key, value, true);
}

SDB_API SdbKv* sdb_ht_find_kvp(SdbPHt* ht, const char* key, bool* found) {
	return (SdbKv *)ht_p_find_kv (ht, key, found);
}

SDB_API char* sdb_ht_find(SdbPHt* ht, const char* key, bool* found) {
	return (char *)ht_p_find (ht, key, found);
}

SDB_API void sdb_ht_free(SdbPHt *ht) {
	ht_p_free (ht);
}

SDB_API bool sdb_ht_delete(SdbPHt* ht, const char *key) {
	return ht_p_delete (ht, key);
}
