#include "json_utils.h"
#include <stdlib.h>
#include <string.h>
#include "sdb/sdb.h"

size_t json_escape_len(const char *s) {
	size_t len = 0;
	for (; *s; s++) {
		if (*s == '"' || *s == '\\') {
			len += 2;
		} else {
			len++;
		}
	}
	return len;
}

char *json_escape(const char *s) {
	size_t len = json_escape_len(s);
	char *res = (char *)sdb_gh_malloc(len + 1);
	if (!res) return NULL;
	char *p = res;
	for (; *s; s++) {
		if (*s == '"' || *s == '\\') {
			*p++ = '\\';
		}
		*p++ = *s;
	}
	*p = '\0';
	return res;
}
