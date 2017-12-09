#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "sdb.h"

mhtkv *mht_getr(mht *m, mhti k);
void mht_init(mht *m, mht_free f);
static mhti mht_get(mht *m, mhti k);

void mht_init(mht *m, mht_free f) {
	memset(m, 0, sizeof (mht));
	m->f = f;
}

void mht_fini(mht *m) {
	int i;
	if (m) {
		for (i = 0; i < MHTSZ; i++) {
			free (m->table[i]);
		}
		mht_init(m, NULL);
	}
}

mhti mht_hash(const char *s) {
	return (mhti)sdb_hash(s);
}

bool mht_set(mht *m, mhti k, mhti v, void *u) {
	const int bucket = k % MHTSZ;
	if (k == MHTNO) {
		return false;
	}
	mhtkv *kv = m->table[bucket];
	if (!kv) {
		kv = calloc (sizeof(mhtkv), 2);
		if (kv) {
			m->table[bucket] = kv;
			kv->k = MHTNO;
			kv->v = MHTNO;
			kv->u = NULL;
			return mht_set(m, k, v, u);
		}
		return false;
	}
	mhtkv *tmp = kv;
	while (kv->k != MHTNO) {
		if (kv->k == k) {
			kv->v = v;
			kv->u = u;
			return true;
		}
		kv++;
	}
	int cursz = (kv - tmp);
	int curln = cursz / sizeof(mhtkv);
	mhtkv *newkv = realloc(tmp, (curln + 2) * sizeof(mhtkv));
	if (newkv) {
		m->table[bucket] = newkv;
		newkv += curln;
		newkv->k = k;
		newkv->v = v;
		newkv->u = u;
		kv++;
		kv->k = MHTNO;
		kv->v = MHTNO;
		kv->u = NULL;
		return false;
	}
	return true;
}

mhtkv *mht_getr(mht *m, mhti k) {
	int bucket = k % MHTSZ;
	mhtkv *kv = m->table[bucket];
	if (kv) {
		while (kv->k != MHTNO) {
			if (kv->k == k) {
				return kv;
			}
			kv++;
		}
	}
	return NULL;
}

mhti mht_get(mht *m, mhti k) {
	mhtkv *kv = mht_getr(m, k);
	return kv? kv->v: MHTNO;
}

void *mht_getu(mht *m, mhti k) {
	mhtkv *kv = mht_getr(m, k);
	return kv? kv->u: NULL;
}

bool mht_add(mht *m, mhti k, mhti v, void *u) {
	return (mht_get(m, k) == MHTNO)
		? mht_set(m, k, v, u)
		: false;
}

bool mht_del(mht *m, mhti k) {
	int bucket = k % MHTSZ;
	if (k == MHTNO) {
		return false;
	}
	mhtkv *kv = m->table[bucket];
	if (kv) {
		while (kv->k != MHTNO) {
			if (kv->k == k) {
				if (m->f) {
					m->f (kv->u);
				}
				mhtkv *n = kv + 1;
				while (n->k != MHTNO) {
					*kv++ = *n++;
				}
				kv->k = MHTNO;
				return true;
			}
			kv++;
		}
	}
	return false;
}

#if 0
static char *mht_str(mht *m, mhti k) {
	// walk all buckets and print the data..... we need a printer for kv->u
	char *res = malloc (1024);
	int bucket = k % MHTSZ;
	mhti *kv = m->table[bucket];
	char *p = res;
	for (i = 0; i < 1024; i++) {
		sprintf (p, "%s%lld", comma, kv->v);
		p += strlen (p);
		kv++;
	}
	return res;
}

static char *mht_str(mht *m) {
	char *res = malloc (1024);
	int bucket = k % MHTSZ;
	mhti *kv = m->table[bucket];
	int i;
	char *p = res;
	for (i = 0; i < MHTSZ; i++) {
		sprintf (p, "%s%lld", comma, kv->v);
		p += strlen (p);
		kv++;
	}
	return res;
}

#endif
int main() {
	mht m;
	mht_init(&m, NULL);
	mht_set(&m, mht_hash("username"), 1024, NULL);
	mht_set(&m, 32, 212, "test");
	mht_del(&m, mht_hash("username"));
	printf ("%d\n", (int)mht_get(&m, mht_hash("username")));
	printf ("%s\n", mht_getu(&m, 32)); //mht_hash("username")));
	mht_fini(&m);
	return 0;
}
