// Simple heap allocator replacing libc malloc/free via sdb global heap interface
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "sdb/sdb.h"
#include "sdb/heap.h"

#if USE_SDB_HEAP
// sbrk is not available on modern darwins
#define CUSTOM_SBRK 1

#if CUSTOM_SBRK
#define SBRK_REGION_SIZE (64 * 1024 * 1024)  // 64 MB

static void *sbrk_base = NULL;
static void *sbrk_end = NULL;
static void *sbrk_current = NULL;

static void *sbrk_init() {
	sbrk_base = mmap (NULL, SBRK_REGION_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE , -1, 0);
	if (sbrk_base == MAP_FAILED) {
		perror ("mmap");
		return NULL;
	}
	sbrk_current = sbrk_base;
	sbrk_end = (char *)sbrk_base + SBRK_REGION_SIZE;
	return sbrk_base;
}

static void *mysbrk(long increment) {
	if (sbrk_base == NULL) {
		if (!sbrk_init ()) {
			return (void *)-1;
		}
	}

	if (increment == 0) {
		return sbrk_current;
	}

	void *prev_break = sbrk_current;
	void *new_break = (char *)sbrk_current + increment;

	if (new_break < sbrk_base || new_break > sbrk_end) {
		errno = ENOMEM;
		return (void *)-1;
	}

	sbrk_current = new_break;
	return prev_break;
}
#else
#define mysbrk sbrk
#endif

// Block header for free list
typedef struct block_t {
	size_t size;
	struct block_t *next;
} block_t;

// Free list head
static block_t *free_list = NULL;

// Align size to 8 bytes
static size_t align8(size_t sz) {
	return (sz + 7) & ~(size_t)7;
}

static void *heap2_realloc(void *data, void *ptr, size_t size) {
	(void)data;
	// Allocation request
	if (!ptr) {
		if (size == 0) {
			return NULL;
		}
		size_t req = align8(size);
		// First-fit free list search
		block_t *prev = NULL, *curr = free_list;
		while (curr) {
			if (curr->size >= req) {
				break;
			}
			prev = curr;
			curr = curr->next;
		}
		if (curr) {
			// Found a free block; remove or split
			if (curr->size >= req + sizeof(block_t) + 8) {
				// Split block
				block_t *split = (block_t *)((char *)(curr + 1) + req);
				split->size = curr->size - req - sizeof(block_t);
				split->next = curr->next;
				curr->size = req;
				if (prev) {
					prev->next = split;
				} else {
					free_list = split;
				}
			} else {
				// Use entire block
				if (prev) {
					prev->next = curr->next;
				} else {
					free_list = curr->next;
				}
			}
			return (void *)(curr + 1);
		}
		// No suitable free block; request from OS
		size_t total = req + sizeof(block_t);
		block_t *b = (block_t *)mysbrk((intptr_t)total);
		if (b == (void *)-1) {
			return NULL;
		}
		b->size = req;
		return (void *)(b + 1);
	}
	// Reallocation or free
	if (size == 0) {
		// free
		block_t *b = (block_t *)ptr - 1;
		b->next = free_list;
		free_list = b;
		return NULL;
	}
	// realloc
	block_t *b = (block_t *)ptr - 1;
	size_t req = align8 (size);
	if (b->size >= req) {
		// existing block is large enough
		return ptr;
	}
	// allocate new block
	void *newp = heap2_realloc (data, NULL, size);
	if (!newp) {
		return NULL;
	}
	memcpy(newp, ptr, b->size);
	// free old block
	b->next = free_list;
	free_list = b;
	return newp;
}

// Finalize heap: reset free list
static void heap2_fini(void *data) {
	(void)data;
	free_list = NULL;
}

static const SdbGlobalHeap heap2_global = {
	heap2_realloc,
	heap2_fini,
	NULL
};
#else
static SdbGlobalHeap Gheap = { NULL, NULL, NULL };
#endif

SDB_API SdbGlobalHeap *sdb_gh(void) {
#if USE_SDB_HEAP
	return (SdbGlobalHeap *)&heap2_global;
#else
	return &Gheap;
#endif
}

// String duplicate using heap allocator
SDB_API char *sdb_strdup(const char *s) {
	size_t n = strlen (s) + 1;
	char *p = (char *)sdb_gh_malloc (n);
	if (p) {
		memcpy(p, s, n);
	}
	return p;
}
