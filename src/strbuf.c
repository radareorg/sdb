/* sdb - MIT - Copyright 2024 - pancake */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "sdb/sdb.h"

SDB_API StrBuf* strbuf_new(void) {
	return (StrBuf*) sdb_gh_calloc (sizeof (StrBuf), 1);
}

SDB_API StrBuf* strbuf_append(StrBuf *sb, const char *str, const int nl) {
	if (!sb || !str || nl < 0) {
		return sb;
	}
	int len = strlen (str);
	if ((sb->len + len + 2) >= sb->size) {
		size_t newsize = sb->size + len + 256;
		char *b = (char *)sdb_gh_realloc (sb->buf, newsize);
		/// TODO perform free and force all callers to update the ref?
		if (!b) {
			return NULL;
		}
		sb->buf = b;
		sb->size = newsize;
	}
	if (sb->buf && str) {
		memcpy (sb->buf + sb->len, str, len);
		sb->len += len;
	}
#define NEWLINE_AFTER_QUERY 1
#if NEWLINE_AFTER_QUERY
	if (sb->buf && nl) {
		sb->buf[sb->len++] = '\n';
		len++;
	}
#endif
	if (sb->buf) {
		sb->buf[sb->len] = 0;
	}
	return sb;
}

SDB_API StrBuf* strbuf_appendf(StrBuf *sb, const int nl, const char *fmt, ...) {
	if (!sb || !fmt) {
		return sb;
	}
	
	va_list ap;
	va_start (ap, fmt);
	
	// First try with a reasonably sized buffer
	char buf[1024];
	int len = vsnprintf (buf, sizeof (buf), fmt, ap);
	va_end (ap);
	
	// If that was enough, just append the string
	if (len >= 0 && len < (int)sizeof(buf)) {
		return strbuf_append (sb, buf, nl);
	}
	
	// If not, allocate a bigger buffer and try again
	va_start (ap, fmt);
	char *newbuf = (char *)sdb_gh_malloc (len + 1);
	if (!newbuf) {
		va_end (ap);
		return NULL;
	}
	vsnprintf (newbuf, len + 1, fmt, ap);
	va_end (ap);
	
	StrBuf *ret = strbuf_append (sb, newbuf, nl);
	sdb_gh_free (newbuf);
	return ret;
}

SDB_API char* strbuf_drain(StrBuf *sb) {
	if (!sb) {
		return NULL;
	}
	if (!sb->buf) {
		sdb_gh_free (sb);
		return NULL;
	}
	char *buf = sb->buf;
	sb->buf = NULL;
	sb->len = 0;
	sb->size = 0;
	sdb_gh_free (sb);
	return buf;
}

SDB_API StrBuf* strbuf_free(StrBuf *sb) {
	if (sb) {
		sdb_gh_free (sb->buf);
		sdb_gh_free (sb);
	}
	return NULL;
}
