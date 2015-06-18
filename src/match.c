/* sdb - LGPLv3 - Copyright 2015 - pancake */

#include "sdb.h"
#include <ctype.h>

static inline int haveSuffix(const char *glob, int glob_len, const char *sfx) {
	const int sfx_len = strlen (sfx);
	return (glob_len>sfx_len && !strcmp (glob+glob_len-sfx_len, sfx));
}

static inline int havePrefix(const char *glob, int glob_len, const char *pfx) {
	const int pfx_len = strlen (pfx);
	return (pfx_len<glob_len && !strncmp (glob, pfx, pfx_len));
}

enum MatchFlag {
	SDB_LIKE_NONE = 0,
	SDB_LIKE_ICASE = 1,
	SDB_LIKE_START = 2,
	SDB_LIKE_END = 4
};

static inline int mycmp(const char *a, const char *b, int n, int any) {
	int i, j;
	for (i=j=0; a[i] && b[j] && j<n; i++) {
		if (tolower(a[i]) == tolower(b[j])) {
			j++;
		} else {
			if (!any) return 0;
			j = 0;
		}
	}
	return any? j!=n: 1;
}

static inline int strstr2(const char *a, const char *b, int n) {
	int i, j;
	for (i=j=0; a[i] && b[j] && j<n; i++) {
		if (a[i] == b[j]) {
			j++;
		} else {
			j = 0;
		}
	}
	return j == n;
}

static inline int compareString(const char *a, const char *b, int blen, int flags) {
	const int start = flags & SDB_LIKE_START;
	const int end = flags & SDB_LIKE_END;
	int alen = strlen (a);
	if (blen > alen)
		return 0;
	if (flags & SDB_LIKE_ICASE) {
		if (start && end) return alen==blen && !mycmp (a, b, blen, 0);
		if (start) return !mycmp (a, b, blen, 0);
		if (end) return !mycmp (a+(alen-blen), b, blen, 0);
		return !mycmp (a, b, blen, 1);
	}
	if (start && end) return alen==blen && !strncmp (a, b, blen);
	if (start) return !strncmp (a, b, blen);
	if (end) return !strncmp (a+(alen-blen), b, blen);
	return strstr2 (a, b, blen);
}

SDB_API int sdb_match (const char *str, const char *glob) {
	int flags = SDB_LIKE_NONE;
	int glob_len;
	int begin = 0;
	if (!str || !glob)
		return 0;
	glob_len = strlen (glob);
	if (haveSuffix (glob, glob_len, "?i")) {
		glob_len -= 2;
		flags |= SDB_LIKE_ICASE;
	}
	if (havePrefix (glob, glob_len, "^")) {
		glob++;
		glob_len--;
		flags |= SDB_LIKE_START;
	}
	if (haveSuffix (glob+begin, glob_len-begin, "$")) {
		glob_len--;
		flags |= SDB_LIKE_END;
	}
	return compareString (str, glob, glob_len, flags);
}
