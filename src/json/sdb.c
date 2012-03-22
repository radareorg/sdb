#include "rangstr.h"
#include "json.h"

/* public sdb api */
char *sdb_json_get (const char *s, const char *p) {
	Rangstr rs = json_get (s, p);
	return rangstr_dup (&rs);
}

