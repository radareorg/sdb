#ifndef SDB_HEAP_H
#define SDB_HEAP_H 1

typedef void *(*SdbHeapRealloc)(void *data, void *ptr, size_t size);
typedef void (*SdbHeapFini)(void *data);

// global heap apis
typedef struct sdb_global_heap_t {
	SdbHeapRealloc realloc;
	// SdbHeapInit init;
	SdbHeapFini fini;
	void *data;
} SdbGlobalHeap;

extern SdbGlobalHeap Gheap;
extern const SdbGlobalHeap sdb_gh_custom; // custom heap allocator
extern const SdbGlobalHeap sdb_gh_libc; // use libc's heap

static inline void sdb_gh_use(const SdbGlobalHeap *gh) {
	if (gh) {
		memcpy (&Gheap, gh, sizeof (SdbGlobalHeap));
	} else {
		memset (&Gheap, 0, sizeof (SdbGlobalHeap));
	}
}

static inline void sdb_gh_fini() {
	if (Gheap.fini) {
		Gheap.fini (Gheap.data);
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
