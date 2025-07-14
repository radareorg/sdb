#include "sdb/sdb.h"
#include "sdb/heap.h"
// Simple heap allocator replacing libc malloc/free via sdb global heap interface
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "sdb/sdb.h"
#include "sdb/sdb.h"
#include "sdb/heap.h"
// sbrk declaration for systems where unistd.h may not declare it
extern void *sbrk(intptr_t increment);

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

// Realloc-based interface: ptr=NULL => malloc, size=0 => free
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
        block_t *b = sbrk(total);
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
    size_t req = align8(size);
    if (b->size >= req) {
        // existing block is large enough
        return ptr;
    }
    // allocate new block
    void *newp = heap2_realloc(data, NULL, size);
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

// Expose global heap2 interface
SDB_API const SdbGlobalHeap sdb_gh_heap2 = {
    heap2_realloc,
    heap2_fini,
    NULL
};
// Global heap instance
static const SdbGlobalHeap heap2_global = {
    heap2_realloc,
    heap2_fini,
    NULL
};

// Return pointer to global heap
SDB_API SdbGlobalHeap *sdb_gh(void) {
    // point to our custom global heap
    return (SdbGlobalHeap *)&heap2_global;
}

// String duplicate using heap allocator
SDB_API char *sdb_strdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char *)sdb_gh_malloc(n);
    if (p) {
        memcpy(p, s, n);
    }
    return p;
}
