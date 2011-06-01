#include "sdb.h"
#include "types.h"

ut64 sdb_getn(sdb *s, const char *key) {
	ut64 n;
	char *v = sdb_get (s, key);
	if (!v) return 0LL;
	sscanf (v, "%lld", &n); // XXX: support other %ll formats and 0x?
	sdb_setn (s, key, n);
	free (v);
	return n;
}

void sdb_setn(sdb *s, const char *key, ut64 v) {
	char b[128];
	sprintf (b, "%lld", v);
	sdb_set (s, key, b);
}

ut64 sdb_inc(sdb *s, const char *key, ut64 n2) {
	ut64 t, n = sdb_getn (s, key);
	t = n + n2;
	if (t == UT64_MAX) {
		return UT64_MAX;
	}
	if ((UT64_MAX-n2)<n) {
		/* overflow */
		return UT64_MAX;
	}
	sdb_setn (s, key, t);
	return n;
}

ut64 sdb_dec(sdb *s, const char *key, ut64 n2) {
	ut64 n = sdb_getn (s, key);
	if (n2>n) {
		/* overflow */
		sdb_setn (s, key, 0);
		return 0;
	}
	sdb_setn (s, key, n-n2);
	return n;
}
