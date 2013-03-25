/* Copyleft 2011-2013 - sdb - pancake */

#include "sdb.h"

const char *sdb_anext(const char *str) {
	return str+strlen (str)+1;
}

char *sdb_astring(char *str, int *hasnext) {
	char *p = strchr (str, SDB_RS);
	if (!p) {
		if (hasnext) *hasnext = 0;
		return str;
	}
	*p = 0;
	if (hasnext) *hasnext = 1;
	return str;
}

char *sdb_aget(Sdb *s, const char *key, int idx) {
	int i, len;
	const char *str = sdb_getc (s, key, 0);
	char *o, *n, *p = (char*)str;
	if (!str || !*str) return NULL;
	if (idx==0) {
		n = strchr (str, SDB_RS);
		if (!n) return strdup (str);
		len = n-str;
		o = malloc (len+1);
		memcpy (o, str, len);
		o[len] = 0;
		return o;
	}
	for (i=0; i<idx; i++) {
		n = strchr (p, SDB_RS);
		if (!n) return NULL;
		p = n+1;
	}
	if (!p) return NULL;
	n = strchr (p, SDB_RS);
	if (!n) return strdup (p);
	len = n-p;
	o = malloc (len+1);
	memcpy (o, p, len);
	o[len] = 0;
	return o;
}

// TODO: done, but there's room for improvements
int sdb_ains(Sdb *s, const char *key, int idx, const char *val) {
	char *x;
	const char *str = sdb_getc (s, key, 0);
	int lnstr, lstr, lval, ret;
	if (!str || !*str)
		return sdb_set (s, key, val, 0);
	lval = strlen (val);
	lstr = strlen (str);
	x = malloc (lval + lstr + 2);
	if (idx==-1) {
		memcpy (x, str, lstr);
		x[lstr] = SDB_RS;
		memcpy (x+lstr+1, val, lval+1);
		ret = sdb_set (s, key, x, 0);
	} else if (idx == 0) {
		memcpy (x, val, lval);
		x[lval] = SDB_RS;
		memcpy (x+lval+1, str, lstr+1);
		ret = sdb_set (s, key, x, 0);
	} else {
		char *nstr = strdup (str);
		char *ptr = (char*)sdb_aindex (nstr, idx);
		if (ptr) {
			*(ptr-1) = 0;
			lnstr = strlen (nstr);
			memcpy (x, nstr, lnstr);
			x[lnstr] = SDB_RS;
			memcpy (x+lnstr+1, val, lval);
			x[lnstr+lval+1] = SDB_RS;
			memcpy (x+lval+2+lnstr, ptr, strlen (ptr)+1);
			ret = sdb_set (s, key, x, 0);
		} else ret = 0;
	}
	free (x);
	return ret;
}

// set/replace
int sdb_aset(Sdb *s, const char *key, int idx, const char *val) {
	char *nstr, *ptr;
	const char *usr, *str = sdb_getc (s, key, 0);
	int len, lnstr, lstr, lval, ret = 0;
	if (!str || !*str)
		return sdb_set (s, key, val, 0);
	len = sdb_alen (str);
	lval = strlen (val);
	lstr = strlen (str);
	if (idx<0 || idx>len) // append
		return sdb_ains (s, key, -1, val);
	nstr = strdup (str);
	ptr = (char *)sdb_aindex (nstr, idx);
	if (ptr) {
		lnstr = strlen (nstr);
		strcpy (ptr, val);
		usr = sdb_aindex (str, idx+1);
		if (usr) {
			strcat (ptr, "\x1e"); // XXX
			strcat (ptr, usr);
		}
		ret = sdb_set (s, key, nstr, 0);
	}
	free (nstr);
	return ret;
}

int sdb_adel(Sdb *s, const char *key, int idx) {
	int i;
	char *p, *n, *str = sdb_get (s, key, 0);
	p = str;
	if (!str || !*str) return 0;
	if (idx<0) idx = sdb_alen (str);
	for (i = 0; i<idx; i++) {
		n = strchr (p, SDB_RS);
		if (n) p = n+1;
		else return 0;;
	}
	n = strchr (p, SDB_RS);
	if (n) strcpy (p, n+1);
	else *p = 0;
	sdb_set (s, key, str, 0);
	free (str);
	return 1;
}

const char *sdb_aindex(const char *str, int idx) {
	int len = 0;
	const char *n, *p = str;
	for (len=0; ; len++) {
		if (len == idx)
			return p;
		n = strchr (p, SDB_RS);
		if (n) p = n+1;
		else break;
	}
	return NULL;
}

// TODO: make static inline?
int sdb_alen(const char *str) {
	int len = 1;
	const char *n, *p = str;
	if (!p|| !*p) return 0;
	for (len=0; ; len++) {
		n = strchr (p, SDB_RS);
		if (n) p = n+1;
		else break;
	}
	if (*p) len++;
	return len;
}

int sdb_alength(Sdb *s, const char *key) {
	const char *str = sdb_getc (s, key, 0);
	return sdb_alen (str);
}

int sdb_alist(Sdb *s, const char *key) {
	int len = 0, hasnext = 1;
	char *list = sdb_get (s, key, 0);
	char *ptr = list;
	hasnext = list && *list;
	while (hasnext) {
		char *str = sdb_astring (ptr, &hasnext);
		// TODO: use callback instead of printf
		printf ("%s\n", str);
		ptr = (char *)sdb_anext (str);
		len++;
	}
	free (list);
	return len;
}
