/* radare - LGPL - Copyright 2011-2018 pancake */

#include "./r_pair.h"
#include <r_util.h>

R_API void r_pair_set_file (RPair*p, const char *file) {
	if (!file || !*file) return;
	if (p->file) free (p->file);
	p->file = strdup (file);
	if (p->sdb) {
		Sdb *sdb = p->sdb;
		sdb->dir = p->file;
	} else eprintf ("no sdb set\n");
}

R_API void r_pair_fini(RPair *p) {
	ht_free (p->ht);
	r_list_free (p->dbs);
	if (p->file) {
		free (p->file);
		// memleak
		//sdb_free (p->sdb);
		p->sdb = NULL;
	}
	free (p->dir);
}

R_API int r_pair_save(RPair *p, const char *f) {
	r_pair_set_file (p, f);
	return r_pair_sync (p);
}

R_API int r_pair_load(RPair *p, const char *f) {
	RPair *p2;
	char *file = (f&&*f)? strdup (f): p->file? strdup (p->file): NULL;
	r_pair_set_file (p, f);
	r_pair_fini (p);
	if (!file) return false;
	p2 = r_pair_new_from_file (file);
	memcpy (p, p2, sizeof (RPair));
	free (p2);
	free (file);
	return true;
}

R_API RPair *r_pair_new (void) {
	RPair *p = R_NEW0 (RPair);
	p->file = NULL;
	p->sdb = sdb_new (NULL, NULL, 0);
	p->ht = ht_new (NULL, NULL, NULL);
	p->dbs = r_list_new ();
	p->dbs->free = (RListFree)sdb_free;
	return p;
}

R_API RPair *r_pair_new_from_file (const char *file) {
	RPair *p = r_pair_new ();
	p->file = strdup (file);
	p->sdb = sdb_new (NULL, file, 0);
	return p;
}

R_API void r_pair_free (RPair *p) {
	if (p==NULL) return;
	r_pair_fini (p);
	free (p);
}

R_API void r_pair_delete (RPair *p, const char *name) {
	Sdb *sdb;
	char *dom, *key = strdup (name);

	dom = (char *)r_str_lchr (key, '.');
	if (dom) {
		key = dom+1;
		*dom = 0;
		dom = 0;
	} else dom = "";
	sdb = ht_find (p->ht, name, NULL);
	if (sdb)
		sdb_unset (sdb, key, 0);
}

static Sdb *pair_sdb_new(RPair *p, const char *dom) {
	Sdb *sdb;
	char *old = NULL;
	if (p->dir) {
		old = r_sys_getdir ();
		r_sys_mkdirp (p->dir);
		r_sys_chdir (p->dir);
	}
	sdb = sdb_new (NULL, dom, 0);
	if (old) {
		r_sys_chdir (old);
		free (old);
	}
	r_list_append (p->dbs, sdb);
	ht_insert (p->ht, dom, sdb);
	return sdb;
}

R_API char *r_pair_get (RPair *p, const char *name) {
	char *dom, *key, *okey;

	if (p->file) {
		return sdb_get (p->sdb, name, NULL);
	}

	key = okey = strdup (name);
	dom = (char*)r_str_lchr (okey, '.');
	if (dom) {
		char *tmp = okey;
		*dom = 0;
		key = dom+1;
		dom = tmp;
	} else dom = "";
	Sdb *sdb = ht_find (p->ht, dom, NULL);
	if (!sdb) {
		sdb = pair_sdb_new (p, dom);
	}
	dom = sdb_get (sdb, key, NULL);
	free (okey);
	return dom;
}

R_API void r_pair_set (RPair *p, const char *name, const char *value) {
	char *dom, *key, *okey;
	Sdb *sdb;

	if (p->file) {
		sdb_set (p->sdb, name, value, 0);
		return;
	}
	key = strdup (name);
	dom = (char*)r_str_lchr (key, '.');
	if (dom) {
		okey = key;
		*dom = 0;
		key = dom+1;
		dom = okey;
	} else {
		dom = "";
	}
	sdb = ht_find (p->ht, dom, NULL);
	if (!sdb) {
		sdb = pair_sdb_new (p, dom);
	}
	sdb_set (sdb, key, value, 0);
}

R_API RList *r_pair_list (RPair *p, const char *domain) {
	Sdb *s;
	if (p->file) s = p->sdb;
	else s = ht_find (p->ht, domain, NULL);
	if (s) {
		RList *list = r_list_new ();
		char key[SDB_MAX_KEY] = {0};
		char *val = NULL;
		list->free = (RListFree)r_pair_item_free;
		sdb_dump_begin (s);
		while (sdb_dump_dupnext (s, key, &val, NULL)) {
			r_list_append (list, r_pair_item_new (key, val));
			// free (key);
			free (val);
		}
		return list;
	}
	return NULL;
}

R_API RPairItem *r_pair_item_new (const char *k, const char *v) {
	RPairItem *i = R_NEW (RPairItem);
	i->k = strdup (k);
	i->v = strdup (v);
	return i;
}

R_API void r_pair_item_free (RPairItem *i) {
	free (i->k);
	free (i->v);
	free (i);
}

R_API void r_pair_set_sync_dir (RPair *p, const char *dir) {
	free (p->dir);
	p->dir = strdup (dir);
}

R_API void r_pair_reset (RPair *p) {
	Sdb *s;
	RListIter *iter;
	r_list_foreach (p->dbs, iter, s)
		sdb_reset (s);
}

R_API int r_pair_sync (RPair *p) {
	Sdb *s;
	char *old = NULL;
	RListIter *iter;
	if (p->file)
		return sdb_sync (p->sdb);
	if (p->dir) {
		old = r_sys_getdir ();
		r_sys_mkdirp (p->dir);
		r_sys_chdir (p->dir);
	}
	r_list_foreach (p->dbs, iter, s) {
		sdb_sync (s);
	}
	if (old) {
		r_sys_chdir (old);
		free (old);
	}
	return true;
}

R_API char* r_pair_serialize(RPair *p, const char *fmt, void *ptr) {
	return NULL;
}

R_API int r_pair_deserialize(RPair *p, ut8 *out, const char *fmt, const char *ptr) {
	return 0;
}
