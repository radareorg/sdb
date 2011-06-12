/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "sdb.h"

// must be deprecated
static ut32 eod, pos; // what about lseek?

sdb* sdb_new (const char *dir, int lock) {
	sdb* s;
	if (lock && !sdb_lock (sdb_lockfile (dir)))
		return NULL;
	s = malloc (sizeof (sdb));
	if (dir) {
		s->dir = strdup (dir);
		s->fd = open (dir, O_RDONLY);
	} else {
		s->dir = NULL;
		s->fd = -1;
	}
	s->ht = r_ht_new ();
	s->lock = lock;
	//s->ht->list->free = (RListFree)sdb_kv_free;
	// if open fails ignore
	cdb_init (&s->db, s->fd);
	cdb_findstart (&s->db);
	return s;
}

void sdb_free (sdb* s) {
	cdb_free (&s->db);
	if (s->lock)
		sdb_unlock (sdb_lockfile (s->dir));
	r_ht_free (s->ht);
	free (s->dir);
	free (s);
}

char *sdb_get (sdb* s, const char *key) {
	char *buf;
	ut32 hash, pos, len;
	SdbKv *kv;

	hash = cdb_hashstr (key);
	kv = (SdbKv*)r_ht_lookup (s->ht, hash);
	if (kv) {
		if (*kv->value) {
			if (kv->expire && sdb_now () > kv->expire) {
				sdb_delete (s, key);
				return NULL;
			}
			return strdup (kv->value);
		}
		return NULL;
	}

	if (s->fd == -1)
		return NULL;
	cdb_findstart (&s->db);

	if (!cdb_findnext (&s->db, hash, key, strlen (key)))
		return NULL;
	pos = cdb_datapos (&s->db);
	len = cdb_datalen (&s->db);
	if (!(buf = malloc (len+1)))
		return NULL;
	cdb_read (&s->db, buf, len, pos);
	buf[len] = 0;
	return buf;
}

int sdb_delete (sdb *s, const char *key) {
	return sdb_set (s, key, "");
}

int sdb_exists (sdb *s, const char *key) {
	SdbKv *kv;
	ut32 hash = cdb_hashstr (key);
	kv = (SdbKv*)r_ht_lookup (s->ht, hash);
	if (kv) return 1;
	if (s->fd == -1)
		return 0;
	cdb_findstart (&s->db);
	if (cdb_findnext (&s->db, hash, key, strlen (key)))
		return 1;
	return 0;
}

struct sdb_kv* sdb_kv_new (const char *k, const char *v) {
	struct sdb_kv *kv = R_NEW (struct sdb_kv);
	strcpy (kv->key, k);
	strcpy (kv->value, v);
	kv->expire = 0LL;
	return kv;
}

void sdb_kv_free (struct sdb_kv *kv) {
	fprintf (stderr, "kv free!\n"); // XXX: just for debugging
	free (kv);
}

int sdb_set (sdb* s, const char *key, const char *val) {
	SdbKv *kv;
	RHashTableEntry *e;
	ut32 hash = cdb_hashstr (key);
	cdb_findstart (&s->db);
	e = r_ht_search (s->ht, hash);
	if (e) {
		if (cdb_findnext (&s->db, hash, key, strlen (key))) {
			kv = e->data;
			strcpy (kv->value, val);
		} else {
			r_ht_remove_entry (s->ht, e);
		}
		return 0;
	}
	r_ht_insert (s->ht, hash, sdb_kv_new (key, val), NULL);
	return 1;
}

// TODO: refactoring hard
int sdb_add (struct cdb_make *c, const char *key, const char *data) {
	return cdb_make_add (c, key, strlen (key), data, strlen (data));
}

int sdb_sync (sdb* s) {
	int fd;
	SdbKv *kv;
	RListIter it, *iter;
	char k[SDB_KEYSIZE];
	char v[SDB_VALUESIZE];
	struct cdb_make c;
	char *ftmp, *f = s->dir;
	if (!f) return 0;
	ftmp = malloc (strlen (f)+5);
	sprintf (ftmp, "%s.tmp", f);
	fd = open (ftmp, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd == -1) {
		fprintf (stderr, "cannot create %s\n", ftmp);
		return -1;
	}
	cdb_make_start (&c, fd);
	sdb_dump_begin (s);
	while (sdb_dump_next (s, k, v)) {
		ut32 hash = cdb_hashstr (k);
		RHashTableEntry *hte = r_ht_search (s->ht, hash);
		if (hte) {
			kv = (SdbKv*)hte->data;
			if (*kv->value) 
				sdb_add (&c, kv->key, kv->value);
			// XXX: This fails if key is dupped
			//else printf ("remove (%s)\n", kv->key);
			r_list_delete (s->ht->list, hte->iter);
			hte->iter = NULL;
			r_ht_remove_entry (s->ht, hte);
		} else if (*v)
			sdb_add (&c, k, v);
	}
	/* append new keyvalues */
	r_list_foreach (s->ht->list, iter, kv) {
	//	printf ("%s=%s\n", kv->key, kv->value);
		if (*kv->value && kv->expire == 0LL) {
			sdb_add (&c, kv->key, kv->value);
		}
		if (kv->expire == 0LL) {
			it.n = iter->n;
			sdb_delete (s, kv->key);
			iter = &it;
		}
	}
//	printf ("db '%s' created\n", f);
	cdb_make_finish (&c);
	fsync (fd);
	close (fd);
	rename (ftmp, f);
	free (ftmp);
	return 0;
}

static ut32 getnum(int fd) {
	char buf[4];
	ut32 ret = 0;
	if (read (fd, buf, 4) != 4)
		return 0;
	pos += 4;
	ut32_unpack (buf, &ret);
	return ret;
}

static int getbytes(int fd, char *b, int len) {
	if (read (fd, b, len) != len)
		return -1;
	pos += len;
	return len;
}

void sdb_dump_begin (sdb* s) {
	if (s->fd != -1) {
		seek_set (s->fd, 0);
		eod = getnum (s->fd);
		pos = 2048;
		seek_set (s->fd, 2048);
	} else eod = pos = 0;
}

// XXX: possible overflow
int sdb_dump_next (sdb* s, char *key, char *value) {
	ut32 dlen, klen;
	if (s->fd==-1 || !getkvlen (s->fd, &klen, &dlen))
		return 0;
	pos += 4;
	if (key && getbytes (s->fd, key, klen)<1)
		return 0;
	if (value && getbytes (s->fd, value, dlen)<1)
		return 0;
	key[klen] = value[dlen] = 0;
	return 1;
}

// XXX: fix 64 bit issue
ut64 sdb_now () {
        struct timeval now;
        gettimeofday (&now, NULL);
        return (ut64)(now.tv_sec);
}

static ut64 expire_adapt (ut64 e) {
	const ut64 month = 30 * 24 * 60 * 60;
	if (e>0 && e<month) e += sdb_now ();
	return e;
}

int sdb_expire(sdb *s, const char *key, ut64 expire) {
	char *buf;
	ut32 hash, pos, len;
	SdbKv *kv;
	hash = cdb_hashstr (key);
	kv = (SdbKv*)r_ht_lookup (s->ht, hash);
	if (kv) {
		if (*kv->value) {
			kv->expire = expire_adapt (expire);
			return 1;
		}
		return 0;
	}
	if (s->fd == -1)
		return 0;
	cdb_findstart (&s->db);
	if (!cdb_findnext (&s->db, hash, key, strlen (key)))
		return 0;
	pos = cdb_datapos (&s->db);
	len = cdb_datalen (&s->db);
	if (!(buf = malloc (len+1)))
		return 0;
	cdb_read (&s->db, buf, len, pos);
	buf[len] = 0;
	sdb_set (s, key, buf);
	free (buf);
	return sdb_expire (s, key, expire); // recursive
}

ut64 sdb_get_expire(sdb *s, const char *key) {
	SdbKv *kv;
	ut32 hash = cdb_hashstr (key);
	kv = (SdbKv*)r_ht_lookup (s->ht, hash);
	if (kv && *kv->value)
		return kv->expire;
	return 0LL;
}

ut32 sdb_hash(const char *s) {
	return cdb_hashstr (s);
}
