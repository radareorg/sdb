#ifndef SDB_HEAP_H
#define SDB_HEAP_H 1

#include <sdb/sdb.h>

// local heap allocator api

// Size 16
typedef struct free_list {
	struct free_list *next;
	struct free_list *prev;
} free_list;
typedef struct sdb_heap_t {
	// Globals
	int *last_address;
	free_list *free_list_start;
	// To reduce number of mmap calls.
	int last_mapped_size; // 1;
} SdbHeap;

typedef void *(*SdbRealloc)(void *data, void *ptr, size_t size);


SDB_API void *sdb_heap_realloc(SdbHeap *heap, void *ptr, int size);
SDB_API void *sdb_heap_malloc(SdbHeap *heap, int size);
static void sdb_heap_free(SdbHeap *heap, void *ptr);

// global heap apis
typedef struct sdb_global_heap_t {
	SdbRealloc realloc;
	void *data;
} SdbGlobalHeap;

extern SdbGlobalHeap Gheap;

static inline void sdb_gh_use(SdbGlobalHeap *gh) {
	if (gh) {
		memcpy (&Gheap, gh, sizeof (SdbGlobalHeap));
	} else {
		memset (&Gheap, 0, sizeof (SdbGlobalHeap));
	}
}

static inline void *sdb_gh_malloc(size_t size) {
	if (Gheap.realloc) {
		return Gheap.realloc (Gheap.data, NULL, size);
	}
	return malloc (size);
}

static inline void *sdb_gh_realloc(void *ptr, size_t size) {
	if (Gheap.realloc) {
		return Gheap.realloc (Gheap.data, ptr, size);
	}
	return realloc (ptr, size);
}

static inline void sdb_gh_free(void *ptr) {
	if (Gheap.realloc) {
		Gheap.realloc (Gheap.data, ptr, 0);
	} else {
		free (ptr);
	}
}

#endif
