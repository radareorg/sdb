#include "memcache.h"

char *mcsdb_client_incr(McSdb *ms, const char *key, ut64 val) {
	return NULL;
}

char *mcsdb_client_decr(McSdb *ms, const char *key, ut64 val) {
	return NULL;
}

void mcsdb_client_set(McSdb *ms, const char *key, ut64 exptime, const char *body) {
}

int mcsdb_client_add(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	return 0;
}

void mcsdb_client_append(McSdb *ms, const char *key, ut64 exptime, const char *body) {
}

void mcsdb_client_prepend(McSdb *ms, const char *key, ut64 exptime, const char *body) {
}

int mcsdb_client_replace(McSdb *ms, const char *key, ut64 exptime, const char *body) {
	return 0;
}

char *mcsdb_client_get (McSdb *ms, const char *key, ut64 *exptime) {
	return NULL;
}

int mcsdb_client_delete(McSdb *ms, const char *key, ut64 exptime) {
	return 0;
}
