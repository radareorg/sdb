/* Copyleft 2012 - sdb (aka SimpleDB) - pancake<nopcode.org> */

#include "sdb.h"

void sdb_ns_free(Sdb *s) {
	// TODO: recursive free
	int i;
	for (i=0; i<MAXNS; i++) {
		Sdb *s2 = s->ns[i].sdb;
		if (s2) sdb_ns_free (s2);
		sdb_free (s2);
		s->ns[i].sdb = NULL;
		s->ns[i].hash = 0;
	}
}

void sdb_ns_init(Sdb *s) {
	int i;
	for(i=0;i<MAXNS;i++) {
		s->ns[i].hash = 0;
		s->ns[i].sdb = NULL;
	}
}

Sdb *sdb_ns(Sdb *s, const char *name) {
	int i, new = -1;
	// s.ns ("user").set ("name.pancake", "MrPancake")
	ut32 hash = sdb_hashstr (name);
	for (i=0; i<MAXNS; i++) {
		if (new == -1 && s->ns[i].sdb == NULL)
			new = i;
		if (!s->ns[i].hash) break;
		if (s->ns[i].hash==hash)
			return s->ns[i].sdb;
	}
	s->ns[new].hash = hash;
	s->ns[new].sdb = sdb_new (name, 0);
	return s->ns[new].sdb;
}

void sdb_ns_sync (Sdb *s, const char *db) {
	if (!db) {
		/* sync all children namespaces */
		int i;
		for(i=0;i<MAXNS;i++) {
			if (!s->ns[i].sdb)
				continue;
			sdb_sync (s->ns[i].sdb);
		}
		return;
	}
	/* sync given db */
	sdb_sync (s);
}
