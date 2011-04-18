/* Copyleft 2011 - sdb (aka SimpleDB) - pancake<nopcode.org> */
#include <stdio.h>
#include <string.h>
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
	s->dir = strdup (dir);
	s->ht = r_ht_new ();
	s->lock = lock;
	//s->ht->list->free = (RListFree)sdb_kv_free;
	s->fd = open (dir, O_RDONLY);
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
	int p;
	char *buf;
	ut32 hash, pos, len;
	SdbKv *kv;

	hash = cdb_hashstr (key);
	kv = (SdbKv*)r_ht_lookup (s->ht, hash);
	if (kv) return strdup (kv->value);

	if (s->fd == -1)
		return NULL;
	cdb_findstart (&s->db);
	p = cdb_findnext (&s->db, hash, key, strlen (key));
	if (p != 1)
		return NULL;
	pos = cdb_datapos (&s->db);
	len = cdb_datalen (&s->db);
	if (!(buf = malloc (len+1)))
		return NULL;
	cdb_read (&s->db, buf, len, pos);
	buf[len] = 0;
	return buf;
}

struct sdb_kv* sdb_kv_new (const char *k, const char *v) {
	struct sdb_kv *kv = R_NEW (struct sdb_kv);
	strcpy (kv->key, k);
	strcpy (kv->value, v);
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
		kv = e->data;
		strcpy (kv->value, val);
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
	RListIter *iter;
	char k[SDB_KEYSIZE];
	char v[SDB_VALUESIZE];
	struct cdb_make c;
	char *f = s->dir;
	char *ftmp = malloc (strlen (f)+5);
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
			SdbKv *kv = (SdbKv*)hte->data;
			if (*kv->value) 
				sdb_add (&c, kv->key, kv->value);
			// XXX: This fails if key is dupped
			//else printf ("remove (%s)\n", kv->key);
			r_list_delete (s->ht->list, hte->iter);
			hte->iter = NULL;
			r_ht_remove_entry (s->ht, hte);
		} else sdb_add (&c, k, v);
	}
	/* append new keyvalues */
	r_list_foreach (s->ht->list, iter, kv) {
	//	printf ("%s=%s\n", kv->key, kv->value);
		sdb_add (&c, kv->key, kv->value);
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
		seek_set(s->fd, 0);
		eod = getnum (s->fd);
		pos = 2048;
		seek_set (s->fd, 2048);
	} else eod = pos = 0;
}

int sdb_dump_next (sdb* s, char *key, char *value) {
	ut32 dlen, klen;
	if (s->fd == -1)
		return 0;
	if (!getkvlen (s->fd, &klen, &dlen))
		return 0;
	pos += 4;
	if (key && getbytes (s->fd, key, klen)<1)
		return 0;
	if (value && getbytes (s->fd, value, dlen)<1)
		return 0;
	key[klen] = value[dlen] = 0;
	return 1;
}
