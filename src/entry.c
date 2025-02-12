#include <sdb/sdb.h>

#ifndef SDB_CUSTOM_HEAP
extern SdbGlobalHeap sdb_gh_custom;
#define SDB_CUSTOM_HEAP sdb_gh_custom
#else
static const SdbGlobalHeap sdb_gh_libc = { NULL, NULL, NULL };
#endif

int main(int argc, const char **argv) {
#if USE_SDB_HEAP
	sdb_gh_use (&SDB_CUSTOM_HEAP);
#else
	sdb_gh_use (&sdb_gh_libc);
#endif
	return sdb_main (argc, argv);
}
