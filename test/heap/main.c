#include <sdb/sdb.h>
#include <mimalloc.h>

static void *mymi(void *data, void *p, size_t s) {
	return mi_realloc (p, s);
}
SdbGlobalHeap sdb_gh_mimalloc = { &mymi, NULL, NULL };

int main(int argc, const char **argv) {
	sdb_gh_use (&sdb_gh_mimalloc);
	return sdb_main (argc, argv);
}
