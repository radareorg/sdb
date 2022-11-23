/* sdb - MIT - Copyright 2022 - pancake */

#include <sdb/sdb.h>
#include <mimalloc.h>

static void *mymi(void *data, void *p, size_t s) {
	if (p) {
		if (s > 0) {
			return mi_realloc (p, s);
		}
		mi_free (p);
		return NULL;
	}
	if (s > 0) {
		return mi_malloc (s);
	}
	// error
	return mi_realloc (p, s);
}

SdbGlobalHeap sdb_gh_mimalloc = { &mymi, NULL, NULL };

int main(int argc, const char **argv) {
	// sdb_gh_use (&sdb_gh_mimalloc);
	sdb_gh_use (&sdb_gh_custom);
	// sdb_gh_use (&sdb_gh_libc);
	return sdb_main (argc, argv);
}
