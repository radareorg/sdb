/* sdb - MIT - Copyright 2011-2016 - pancake */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "sdb.h"

static inline int nextcas() {
	static ut32 cas = 1;
	if (!cas) {
		cas++;
	}
	return cas++;
}

static SdbHook global_hook = NULL;
static void* global_user = NULL;

SDB_API void sdb_global_hook(SdbHook hook, void *user) {
	global_hook = hook;
	global_user = user;
}

// TODO: use mmap instead of read.. much faster!
SDB_API Sdb* sdb_new0 () {
	return sdb_new (NULL, NULL, 0);
}

SDB_API Sdb* sdb_new (const char *path, const char *name, int lock) {
	Sdb* s = R_NEW0 (Sdb);
	if (!s) {
		return NULL;
	}
	s->fd = -1;
	s->refs = 1;
	if (path && !*path) {
		path = NULL;
	}
	if (name && *name && strcmp (name, "-")) {
		if (path && *path) {
			int plen = strlen (path);
			int nlen = strlen (name);
			s->dir = malloc (plen + nlen + 2);
			if (!s->dir) {
				free (s);
				return NULL;
			}
			memcpy (s->dir, path, plen);
			s->dir[plen] = '/';
			memcpy (s->dir + plen + 1, name, nlen + 1);
			s->path = strdup (path);
		} else {
			s->dir = strdup (name);
		}
		switch (lock) {
		case 1:
			if (!sdb_lock (sdb_lock_file (s->dir))) {
				goto fail;
			}
			break;
		case 2:
			if (!sdb_lock_wait (sdb_lock_file (s->dir))) {
				goto fail;
			}
			break;
		}
		if (sdb_open (s, s->dir) == -1) {
			s->last = sdb_now ();
			// TODO: must fail if we cant open for write in sync
		}
		s->name = strdup (name);
	} else {
		s->last = sdb_now ();
		s->fd = -1;
	}
	s->journal = -1;
	s->fdump = -1;
	s->depth = 0;
	s->ndump = NULL;
	s->ns = ls_new (); // TODO: should be NULL
	if (!s->ns) {
		goto fail;
	}
	s->ns->free = NULL;
	if (!s->ns) {
		goto fail;
	}
	s->ht = ht_new ();
	s->lock = lock;
	// s->ht->list->free = (SdbListFree)sdb_kv_free;
	// if open fails ignore
	if (global_hook) {
		sdb_hook (s, global_hook, global_user);
	}
	cdb_init (&s->db, s->fd);
	return s;
fail:
	if (s->fd != -1) {
		close (s->fd);
		s->fd = -1;
	}
	free (s->dir);
	free (s->name);
	free (s->path);
	free (s);
	return NULL;
}

// XXX: this is wrong. stuff not stored in memory is lost
SDB_API void sdb_file (Sdb* s, const char *dir) {
	if (s->lock) {
		sdb_unlock (sdb_lock_file (s->dir));
	}
	free (s->dir);
	s->dir = (dir && *dir)? strdup (dir): NULL;
	if (s->lock) {
		sdb_lock (sdb_lock_file (s->dir));
	}
}

static int sdb_merge_cb(void *user, const char *k, const char *v) {
	sdb_set (user, k, v, 0);
	return true;
}

SDB_API bool sdb_merge(Sdb* d, Sdb *s) {
	return sdb_foreach (s, sdb_merge_cb, d);
}

SDB_API int sdb_count(Sdb *s) {
	int count = 0;
	if (s) {
		if (s->db.fd != -1) {
			sdb_dump_begin (s);
			while (sdb_dump_hasnext (s)) {
				count++;
			}
		}
		if (s->ht) {
			count += s->ht->count;
		}
	}
	return count;
}

static void sdb_fini(Sdb* s, int donull) {
	if (!s) return;
	sdb_hook_free (s);
	cdb_free (&s->db);
	if (s->lock) {
		sdb_unlock (sdb_lock_file (s->dir));
	}
	sdb_ns_free (s);
	s->refs = 0;
	free (s->name);
	free (s->path);
	ls_free (s->ns);
	ht_free (s->ht);
	sdb_journal_close (s);
	if (s->fd != -1) {
		close (s->fd);
		s->fd = -1;
	}
	free (s->ndump);
	free (s->dir);
	free (s->tmpkv.value);
	s->tmpkv.value_len = 0;
	if (donull) {
		memset (s, 0, sizeof (Sdb));
	}
}

SDB_API bool sdb_free (Sdb* s) {
	if (s && s->ht && s->refs) {
		s->refs--;
		if (s->refs < 1) {
			s->refs = 0;
			sdb_fini (s, 0);
			s->ht = NULL;
			free (s);
			return true;
		}
	}
	return false;
}

SDB_API const char *sdb_const_get_len (Sdb* s, const char *key, int *vlen, ut32 *cas) {
	ut32 pos, len, keylen;
	ut64 now = 0LL;
	SdbKv *kv;
	bool found;

	if (cas) {
		*cas = 0;
	}
	if (vlen) {
		*vlen = 0;
	}
	if (!s || !key) {
		return NULL;
	}
	// TODO: optimize, iterate once
	keylen = strlen (key);

	/* search in memory */
	kv = (SdbKv*) ht_find_kvp (s->ht, key, &found);
	if (found) {
		if (!kv->value || !*kv->value) {
			return NULL;
		}
		if (kv->expire) {
			if (!now) {
				now = sdb_now ();
			}
			if (now > kv->expire) {
				sdb_unset (s, key, 0);
				return NULL;
			}
		}
		if (cas) {
			*cas = kv->cas;
		}
		if (vlen) {
			*vlen = kv->value_len;
		}
		return kv->value;
	}
	/* search in disk */
	if (s->fd == -1) {
		return NULL;
	}
	(void) cdb_findstart (&s->db);
	if (cdb_findnext (&s->db, s->ht->hashfn (key), key, keylen) < 1) {
		return NULL;
	}
	len = cdb_datalen (&s->db);
	if (len < SDB_MIN_VALUE || len >= SDB_MAX_VALUE) {
		return NULL;
	}
	if (vlen) {
		*vlen = len;
	}
	pos = cdb_datapos (&s->db);
	return s->db.map + pos;
}

SDB_API const char *sdb_const_get (Sdb* s, const char *key, ut32 *cas) {
	return sdb_const_get_len (s, key, NULL, cas);
}

// TODO: add sdb_getf?

SDB_API char *sdb_get_len (Sdb* s, const char *key, int *vlen, ut32 *cas) {
	const char *value = sdb_const_get_len (s, key, vlen, cas);
	return value ? strdup (value) : NULL;
}

SDB_API char *sdb_get (Sdb* s, const char *key, ut32 *cas) {
	return sdb_get_len (s, key, NULL, cas);
}

SDB_API int sdb_unset (Sdb* s, const char *key, ut32 cas) {
	return key? sdb_set (s, key, "", cas): 0;
}

/* remove from memory */
SDB_API bool sdb_remove(Sdb *s, const char *key, ut32 cas) {
	return ht_delete (s->ht, key);
}

// alias for '-key=str'.. '+key=str' concats
SDB_API int sdb_uncat(Sdb *s, const char *key, const char *value, ut32 cas) {
	// remove 'value' from current key value.
	// TODO: cas is ignored here
	int vlen = 0, valen;
	char *p, *v = sdb_get_len (s, key, &vlen, NULL);
	int mod = 0;
	if (!v || !key || !value) {
		return 0;
	}
	valen = strlen (value);
	if (valen > 0) {
		while ((p = strstr (v, value))) {
			memmove (p, p + valen, strlen (p + valen) + 1);
			mod = 1;
		}
	}
	if (mod) {
		sdb_set_owned (s, key, v, 0);
	} else {
		free (v);
	}
	return 0;
}

SDB_API int sdb_concat(Sdb *s, const char *key, const char *value, ut32 cas) {
	int kl, vl;
	const char *p;
	char *o;
	if (!s || !key || !*key || !value || !*value) {
		return 0;
	}
	p = sdb_const_get_len (s, key, &kl, 0);
	if (!p) {
		return sdb_set (s, key, value, cas);
	}
	vl = strlen (value);
	o = malloc (kl + vl + 1);
	if (o) {
		memcpy (o, p, kl);
		memcpy (o + kl, value, vl + 1);
		return sdb_set_owned (s, key, o, cas);
	}
	return 0;
}

// set if not defined
SDB_API int sdb_add (Sdb* s, const char *key, const char *val, ut32 cas) {
	if (sdb_exists (s, key)) {
		return 0;
	}
	return sdb_set (s, key, val, cas);
}

SDB_API bool sdb_exists (Sdb* s, const char *key) {
	ut32 pos;
	char ch;
	SdbKv *kv;
	bool found;
	int klen = strlen (key) + 1;
	if (!s) {
		return false;
	}
	kv = (SdbKv*)ht_find_kvp (s->ht, key, &found);
	if (found && kv) {
		return *kv->value;
	}
	if (s->fd == -1) {
		return false;
	}
	(void)cdb_findstart (&s->db);
	if (cdb_findnext (&s->db, sdb_hash (key), key, klen)) {
		pos = cdb_datapos (&s->db);
		cdb_read (&s->db, &ch, 1, pos);
		return ch != 0;
	}
	return false;
}

SDB_API int sdb_open (Sdb *s, const char *file) {
        struct stat st;
	if (!s) {
		return -1;
	}
	if (file) {
		if (s->fd != -1) {
			close (s->fd);
			s->fd = -1;
		}
		s->fd = open (file, O_RDONLY | O_BINARY);
		if (file != s->dir) {
			free (s->dir);
			s->dir = strdup (file);
			s->path = NULL; // TODO: path is important
		}
	}
	s->last = 0LL;
	if (s->fd != -1 && fstat (s->fd, &st) != -1) {
		if ((S_IFREG & st.st_mode) != S_IFREG) {
			eprintf ("Database must be a file\n");
			close (s->fd);
			s->fd = -1;
			return -1;
		}
		s->last = st.st_mtime;
	}
	if (s->fd != -1) {
		cdb_init (&s->db, s->fd);
	}
	return s->fd;
}

SDB_API void sdb_close (Sdb *s) {
	if (s) {
		if (s->fd != -1) {
			close (s->fd);
			s->fd = -1;
		}
		if (s->dir) {
			free (s->dir);
			s->dir = NULL;
		}
	}
}

SDB_API void sdb_reset(Sdb* s) {
	if (!s) {
		return;
	}
	/* ignore disk cache, file is not removed, but we will ignore
	 * its values when syncing again */
	sdb_close (s);
	/* empty memory hashtable */
	ht_free (s->ht);
	s->ht = ht_new ();
}

static int lastIndex(const char *str) {
	int len = strlen (str);
	return (len > 0)? len - 1: 0;
}

static bool match(const char *str, const char *expr) {
	bool startsWith = *expr == '^';
	bool endsWith = expr[lastIndex (expr)] == '$';
	if (startsWith && endsWith) {
		return !strncmp (str, expr + 1, strlen (expr) - 2);
	}
	if (startsWith) {
		return !strncmp (str, expr + 1, strlen (expr) - 1);
	}
	if (endsWith) {
		int alen = strlen (str);
		int blen = strlen (expr) - 1;
		if (alen <= blen) {
			return false;
		}
		char *a = str + strlen (str) - blen;
		return (!strncmp (a, expr, blen));
	}
	return strstr (str, expr);
}

SDB_API bool sdb_kv_match(SdbKv *kv, const char *expr) {
	// TODO: add syntax to negate condition
	// TODO: add syntax to OR k/v instead of AND
	// [^]str[$]=[^]str[$]
	const char *eq = strchr (expr, '=');
	if (eq) {
		char *e = strdup (expr);
		char *ep = e + (eq - expr);
		*ep++ = 0;
		bool res = !*e || match (kv->key, e);
		bool res2 = !*ep || match (kv->value, ep);
		free (e);
		return res && res2;
	}
	return match (kv->key, expr);
}

SDB_API SdbKv* sdb_kv_new(const char *k, const char *v) {
	SdbKv *kv;
	int vl;
	if (!sdb_check_key (k)) {
		return NULL;
	}
	if (v) {
		if (!sdb_check_value (v)) {
			return NULL;
		}
		vl = strlen (v);
	} else {
		vl = 0;
	}
	kv = R_NEW0 (SdbKv);
	kv->key_len = strlen (k);
	kv->key = malloc (kv->key_len + 1);
	memcpy (kv->key, k, kv->key_len + 1);
	kv->value_len = vl;
	if (vl) {
		kv->value = malloc (vl + 1);
		if (!kv->value) {
			free (kv);
			return NULL;
		}
		memcpy (kv->value, v, vl + 1);
	} else {
		kv->value = NULL;
		kv->value_len = 0;
	}
	kv->cas = nextcas ();
	kv->expire = 0LL;
	return kv;
}

SDB_API void sdb_kv_free (SdbKv *kv) {
	free (kv->key);
	free (kv->value);
	free (kv);
}

static int sdb_set_internal (Sdb* s, const char *key, char *val, int owned, ut32 cas) {
	ut32 vlen, klen;
	SdbKv *kv;
	bool found;
	if (!s || !sdb_check_key (key)) {
		return 0;
	}
	if (!val) {
		val = "";
	}
	/* 100ms */
	if (!sdb_check_value (val)) {
		return 0;
	}
	if (s->journal != -1) {
		sdb_journal_log (s, key, val);
	}
	klen = strlen (key);
	vlen = strlen (val);
	cdb_findstart (&s->db);
	kv = ht_find_kvp (s->ht, key, &found);
	if (found && kv->value) {
		if (cdb_findnext (&s->db, sdb_hash (key), key, klen)) {
			if (cas && kv->cas != cas) {
				return 0;
			}
			if (vlen == kv->value_len && !strcmp (kv->value, val)) {
				return kv->cas;
			}
			kv->cas = cas = nextcas ();
			if (owned) {
				kv->value_len = vlen;
				free (kv->value);
				kv->value = val; // owned
			} else {
				if ((ut32)vlen > kv->value_len) {
					free (kv->value);
					kv->value = strdup (val);
				} else {
					memcpy (kv->value, val, vlen + 1);
				}
				kv->value_len = vlen;
			}
		} else {
			ht_delete (s->ht, key);
		}
		sdb_hook_call (s, key, val);
		return cas;
	}
	// empty values are also stored
	// TODO store only the ones that are in the CDB
	if (owned) {
		kv = sdb_kv_new (key, NULL);
		if (kv) {
			kv->value = val;
			kv->value_len = vlen;
		}
	} else {
		kv = sdb_kv_new (key, val);
	}
	if (kv) {
		ut32 cas = kv->cas = nextcas ();
		ht_insert_kvp (s->ht, kv, true /*update*/);
		sdb_hook_call (s, key, val);
		return cas;
	}
	sdb_hook_call (s, key, val);
	return 0;
}

SDB_API int sdb_set_owned (Sdb* s, const char *key, char *val, ut32 cas) {
	return sdb_set_internal (s, key, val, 1, cas);
}

SDB_API int sdb_set (Sdb* s, const char *key, const char *val, ut32 cas) {
	return sdb_set_internal (s, key, (char*)val, 0, cas);
}

static int sdb_foreach_list_cb(void *user, const char *k, const char *v) {
	SdbList *list = (SdbList *)user;
	SdbKv *kv = R_NEW0 (SdbKv);
	/* seems like some k/v are constructed in the stack and cant be used after returning */
	kv->key = strdup (k);
	kv->value = strdup (v);
	ls_append (list, kv);
	return 1;
}

static int __cmp_asc(const void *a, const void *b) {
	const SdbKv *ka = a;
	const SdbKv *kb = b;
	return strcmp (ka->key, kb->key);
}

SDB_API SdbList *sdb_foreach_list (Sdb* s, bool sorted) {
	SdbList *list = ls_newf ((SdbListFree)sdb_kv_free);
	sdb_foreach (s, sdb_foreach_list_cb, list);
	if (sorted) {
		ls_sort (list, __cmp_asc);
	}
	return list;
}

typedef struct {
	const char *expr;
	SdbList *list;
	bool single;
} _;

static int sdb_foreach_match_cb(void *user, const char *k, const char *v) {
	_ *o = (_*)user;
	SdbKv tkv = { .key = k, .value = v };
	if (sdb_kv_match (&tkv, o->expr)) {
		SdbKv *kv = R_NEW0 (SdbKv);
		kv->key = strdup (k);
		kv->value = strdup (v);
		ls_append (o->list, kv);
		if (o->single) {
			return 0;
		}
	}
	return 1;
}

SDB_API SdbList *sdb_foreach_match (Sdb* s, const char *expr, bool single) {
	SdbList *list = ls_newf ((SdbListFree)sdb_kv_free);
	_ o = { expr, list, single };
	sdb_foreach (s, sdb_foreach_match_cb, &o);
#if 0
	// TODO. add sorted ? wtf
	if (sorted) {
		ls_sort (list, __cmp_asc);
	}
#endif
	return list;
}

static bool sdb_foreach_end (Sdb *s, bool result) {
	s->depth--;
	if (s->depth == 0) {
		ht_free_deleted (s->ht);
	}
	return result;
}

SDB_API bool sdb_foreach (Sdb* s, SdbForeachCallback cb, void *user) {
	SdbListIter *iter;
	char *k, *v;
	SdbKv *kv;
	bool found;
	ut32 i;
	if (!s) {
		return false;
	}
	s->depth++;
	sdb_dump_begin (s);
	while (sdb_dump_dupnext (s, &k, &v, NULL)) {
		SdbKv *kv = ht_find_kvp (s->ht, k, &found);
		// TODO avoid using the heap for k/v allocations
		if (found) {
			free (k);
			free (v);
			if (*kv->key && *kv->value) {
				if (!cb (user, kv->key, kv->value)) {
					return sdb_foreach_end (s, false);
				}
			}
		} else {
			if (!cb (user, k, v)) {
				free (k);
				free (v);
				return sdb_foreach_end (s, false);
			}
			free (k);
			free (v);
		}
	}
	for (i = 0; i < s->ht->size; i++) {
		ls_foreach (s->ht->table[i], iter, kv) {
			if (!kv || !kv->value || !*kv->value) {
				continue;
			}
			if (!cb (user, kv->key, kv->value)) {
				return sdb_foreach_end (s, false);
			}
		}
	}
	return sdb_foreach_end (s, true);
}

SDB_API bool sdb_sync (Sdb* s) {
	SdbListIter it, *iter;
	char *k, *v;
	SdbKv *kv;
	bool found;
	ut32 i;

	if (!s || !sdb_disk_create (s)) {
		return false;
	}
// TODO: use sdb_foreach here
	sdb_dump_begin (s);
	/* iterate over all keys in disk database */
	while (sdb_dump_dupnext (s, &k, &v, NULL)) {
		/* find that key in the memory storage */
		kv = ht_find_kvp (s->ht, k, &found);
		if (found) {
			if (kv && kv->value && *kv->value) {
				/* asume k = kv->key */
				sdb_disk_insert (s, k, kv->value);
			}
			// XXX: This fails if key is dupped
			//else printf ("remove (%s)\n", kv->key);
			ht_delete (s->ht, k);
		} else if (v && *v) {
			sdb_disk_insert (s, k, v);
		}
		free (k);
		free (v);
	}
	/* append new keyvalues */
	for (i = 0; i < s->ht->size; ++i) {
		ls_foreach (s->ht->table[i], iter, kv) {
			if (kv->key && kv->value && *kv->value && kv->expire == 0LL) {
				if (sdb_disk_insert (s, kv->key, kv->value)) {
					it.n = iter->n;
					//sdb_unset (s, kv->key, 0);
					sdb_remove (s, kv->key, 0);
					iter = &it;
				}
			}
		}
	}
	sdb_disk_finish (s);
	sdb_journal_clear (s);
	// TODO: sdb_reset memory state?
	return true;
}

// TODO: optimize: do not use syscalls here. we can just do mmap and pointer arithmetics
static int getbytes(Sdb *s, char *b, int len) {
	if (read (s->fd, b, len) != len) {
		return -1;
	}
	s->pos += len;
	return len;
}

SDB_API void sdb_dump_begin (Sdb* s) {
	if (s->fd != -1) {
		s->pos = sizeof (((struct cdb_make *)0)->final);
		seek_set (s->fd, s->pos);
	} else {
		s->pos = 0;
	}
}

SDB_API SdbKv *sdb_dump_next (Sdb* s) {
	char *k = NULL, *v = NULL;
	int vl = 0;
	// we dont need to malloc, because all values are null terminated in memory.
	if (!sdb_dump_dupnext (s, &k, &v, &vl)) {
		return NULL;
	}
	vl--;
	strncpy (s->tmpkv.key, k, SDB_KSZ - 1);
	s->tmpkv.key[SDB_KSZ - 1] = '\0';
	free (k);
	free (s->tmpkv.value);
	s->tmpkv.value = v;
	s->tmpkv.value_len = vl;
	return &s->tmpkv;
}

SDB_API bool sdb_dump_hasnext (Sdb* s) {
	ut32 k, v;
	if (s->fd == -1) {
		return false;
	}
	if (!cdb_getkvlen (s->fd, &k, &v)) {
		return false;
	}
	if (k < 1 || v < 1) {
		return false;
	}
	if (lseek (s->fd, k + v, SEEK_CUR) == -1) {
		return false;
	}
	s->pos += k + v + 4;
	return true;
}

SDB_API bool sdb_stats(Sdb *s, ut32 *disk, ut32 *mem) {
	if (!s) {
		return false;
	}
	if (disk) {
		ut32 count = 0;
		if (s->fd != -1) {
			sdb_dump_begin (s);
			while (sdb_dump_hasnext (s)) {
				count ++;
			}
		}
		*disk = count;
	}
	if (mem) {
		*mem = s->ht->count;
	}
	return disk || mem;
}

// TODO: make it static? internal api?
SDB_API bool sdb_dump_dupnext (Sdb* s, char **key, char **value, int *_vlen) {
	ut32 vlen = 0, klen = 0;
	if (key) {
		*key = NULL;
	}
	if (value) {
		*value = NULL;
	}
	if (_vlen) {
		*_vlen = 0;
	}
	if (s->fd == -1) {
		return false;
	}
	if (!cdb_getkvlen (s->fd, &klen, &vlen)) {
		return false;
	}
	if (klen < 1 || vlen < 1) {
		return false;
	}
	if (_vlen) {
		*_vlen = vlen;
	}
	if (key) {
		*key = 0;
		if (klen > SDB_MIN_KEY && klen < SDB_MAX_KEY) {
			*key = malloc (klen + 1);
			if (!*key) {
				return 0;
			}
			if (getbytes (s, *key, klen) == -1) {
				free (*key);
				*key = NULL;
				return 0;
			}
			(*key)[klen] = 0;
		}
	}
	if (value) {
		*value = 0;
		if (vlen >= SDB_MIN_VALUE && vlen < SDB_MAX_VALUE) {
			*value = malloc (vlen + 10);
			if (!*value) {
				if (key) {
					free (*key);
					*key = NULL;
				}
				return false;
			}
			if (getbytes (s, *value, vlen) == -1) {
				if (key) {
					free (*key);
					*key = NULL;
				}
				free (*value);
				*value = NULL;
				return false;
			}
			(*value)[vlen] = 0;
		}
	}
	s->pos += 4; // XXX no.
	return true;
}

static inline ut64 parse_expire (ut64 e) {
	const ut64 month = 30 * 24 * 60 * 60;
	if (e > 0 && e < month) {
		e += sdb_now ();
	}
	return e;
}

SDB_API bool sdb_expire_set(Sdb* s, const char *key, ut64 expire, ut32 cas) {
	char *buf;
	ut32 pos, len;
	SdbKv *kv;
	bool found;
	if (!key) {
		s->expire = parse_expire (expire);
		return true;
	}
	kv = (SdbKv*)ht_find_kvp (s->ht, key, &found);
	if (found && kv) {
		if (*kv->value) {
			if (!cas || cas == kv->cas) {
				kv->expire = parse_expire (expire);
				return true;
			}
		}
		return false;
	}
	if (s->fd == -1) {
		return false;
	}
	(void) cdb_findstart (&s->db);
	if (!cdb_findnext (&s->db, sdb_hash (key), key, strlen (key) + 1)) {
		return false;
	}
	pos = cdb_datapos (&s->db);
	len = cdb_datalen (&s->db);
	if (len < 1 || len == UT32_MAX) {
		return false;
	}
	if (!(buf = calloc (1, len + 1))) {
		return false;
	}
	cdb_read (&s->db, buf, len, pos);
	buf[len] = 0;
	sdb_set_owned (s, key, buf, cas);
	return sdb_expire_set (s, key, expire, cas); // recursive
}

SDB_API ut64 sdb_expire_get(Sdb* s, const char *key, ut32 *cas) {
	bool found = false;
	SdbKv *kv = (SdbKv*)ht_find_kvp (s->ht, key, &found);
	if (found && kv && *kv->value) {
		if (cas) {
			*cas = kv->cas;
		}
		return kv->expire;
	}
	return 0LL;
}

SDB_API bool sdb_hook(Sdb* s, SdbHook cb, void* user) {
	int i = 0;
	SdbHook hook;
	SdbListIter *iter;
	if (s->hooks) {
		ls_foreach (s->hooks, iter, hook) {
			if (!(i % 2) && (hook == cb)) {
				return false;
			}
			i++;
		}
	} else {
		s->hooks = ls_new ();
		s->hooks->free = NULL;
	}
	ls_append (s->hooks, cb);
	ls_append (s->hooks, user);
	return true;
}

SDB_API bool sdb_unhook(Sdb* s, SdbHook h) {
	int i = 0;
	SdbHook hook;
	SdbListIter *iter, *iter2;
	ls_foreach (s->hooks, iter, hook) {
		if (!(i%2) && (hook == h)) {
			iter2 = iter->n;
			ls_delete (s->hooks, iter);
			ls_delete (s->hooks, iter2);
			return true;
		}
		i++;
	}
	return false;
}

SDB_API int sdb_hook_call(Sdb *s, const char *k, const char *v) {
	SdbListIter *iter;
	SdbHook hook;
	int i = 0;
	if (s->last) {
		s->last = sdb_now ();
	}
	ls_foreach (s->hooks, iter, hook) {
		if (!(i % 2) && k && iter->n) {
			void *u = iter->n->data;
			hook (s, u, k, v);
		}
		i++;
	}
	return i >> 1;
}

SDB_API void sdb_hook_free(Sdb *s) {
	ls_free (s->hooks);
	s->hooks = NULL;
}

SDB_API void sdb_config(Sdb *s, int options) {
	s->options = options;
	if (options & SDB_OPTION_SYNC) {
		// sync on every query
	}
	if (options & SDB_OPTION_JOURNAL) {
		// sync on every query
		sdb_journal_open (s);
		// load journaling if exists
		sdb_journal_load (s);
		sdb_journal_clear (s);
	} else {
		sdb_journal_close (s);
	}
	if (options & SDB_OPTION_NOSTAMP) {
		// sync on every query
		s->last = 0LL;
	}
	if (options & SDB_OPTION_FS) {
		// have access to fs (handle '.' or not in query)
	}
}

SDB_API int sdb_unlink (Sdb* s) {
	sdb_fini (s, 1);
	return sdb_disk_unlink (s);
}

SDB_API void sdb_drain(Sdb *s, Sdb *f) {
	if (s && f) {
		f->refs = s->refs;
		sdb_fini (s, 1);
		*s = *f;
		free (f);
	}
}

typedef struct {
	Sdb *sdb;
	const char *key;
} UnsetCallbackData;

static int unset_cb(void *user, const char *k, const char *v) {
	UnsetCallbackData *ucd = user;
	if (sdb_match (k, ucd->key)) {
		sdb_unset (ucd->sdb, k, 0);
	}
	return 1;
}

SDB_API int sdb_unset_like(Sdb *s, const char *k) {
	UnsetCallbackData ucd = { s, k };
	return sdb_foreach (s, unset_cb, &ucd);
}

typedef struct {
	Sdb *sdb;
	const char *key;
	const char *val;
	SdbForeachCallback cb;
	const char **array;
	int array_index;
	int array_size;
} LikeCallbackData;

static int like_cb(void *user, const char *k, const char *v) {
	LikeCallbackData *lcd = user;
	if (!user) {
		return 0;
	}
	if (k && lcd->key && !sdb_match (k, lcd->key)) {
		return 1;
	}
	if (v && lcd->val && !sdb_match (v, lcd->val)) {
		return 1;
	}
	if (lcd->array) {
		int idx = lcd->array_index;
		int newsize = lcd->array_size + sizeof (char*) * 2;
		const char **newarray = realloc (lcd->array, newsize);
		if (!newarray) {
			return 0;
		}
		lcd->array = newarray;
		lcd->array_size = newsize;
		// concatenate in array
		lcd->array[idx] = k;
		lcd->array[idx + 1] = v;
		lcd->array[idx + 2] = NULL;
		lcd->array[idx + 3] = NULL;
		lcd->array_index = idx+2;
	} else {
		if (lcd->cb) {
			lcd->cb (lcd->sdb, k, v);
		}
	}
	return 1;
}

SDB_API char** sdb_like(Sdb *s, const char *k, const char *v, SdbForeachCallback cb) {
	LikeCallbackData lcd = { s, k, v, cb, NULL, 0, 0 };
	if (cb) {
		sdb_foreach (s, like_cb, &lcd);
		return NULL;
	}
	if (k && !*k) {
		lcd.key = NULL;
	}
	if (v && !*v) {
		lcd.val = NULL;
	}
	lcd.array_size = sizeof (char*) * 2;
	lcd.array = calloc (lcd.array_size, 1);
	if (!lcd.array) {
		return NULL;
	}
	lcd.array_index = 0;
	sdb_foreach (s, like_cb, &lcd);
	if (lcd.array_index == 0) {
		free (lcd.array);
		return NULL;
	}
	return (char**)lcd.array;
}
