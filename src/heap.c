#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "sdb/sdb.h"
#include "sdb/heap.h"

#if USE_MMAN
#if __SDB_WINDOWS__
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>

#if !defined(MAP_ANONYMOUS)
#if __MACH__
  #define MAP_ANONYMOUS 0x1000
#else
  #define MAP_ANONYMOUS 0x20
#endif
#endif

// Define constants for alignment and page size
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SDB_PAGE_SIZE 4096
#define MIN_ALLOC_SIZE 32

// Block header structure
typedef struct BlockHeader {
    size_t size;         // Size of the block (including header)
    bool free;           // Whether the block is free
    struct BlockHeader *next;  // Next block in memory
    struct BlockHeader *prev;  // Previous block in memory
} BlockHeader;

// Free list structure to track available blocks
typedef struct FreeBlock {
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct SdbHeap {
    BlockHeader *head;       // First block in memory
    FreeBlock *free_list;    // List of free blocks
    size_t total_size;       // Total allocated memory size
    size_t used_size;        // Currently used memory size
} SdbHeap;

static SdbHeap sdb_heap_custom_data = {NULL, NULL, 0, 0};
static SdbGlobalHeap Gheap = { NULL, NULL, NULL };

// Initialize the global heap
SDB_API SdbGlobalHeap *sdb_gh(void) {
    return &Gheap;
}

SDB_API char *sdb_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = (char *)sdb_gh_malloc(len);
    if (p) {
        memcpy(p, s, len);
    }
    return p;
}

// Get the header from a user pointer
static BlockHeader *get_header(void *ptr) {
    return ptr ? (BlockHeader *)((char *)ptr - sizeof(BlockHeader)) : NULL;
}

// Get the user pointer from a header
static void *get_user_ptr(BlockHeader *header) {
    return header ? (void *)((char *)header + sizeof(BlockHeader)) : NULL;
}

// Insert a block into the free list
static void insert_free_block(SdbHeap *heap, BlockHeader *block) {
    if (!block || !block->free) {
        return;
    }

    FreeBlock *free_block = (FreeBlock *)get_user_ptr(block);
    free_block->next = heap->free_list;
    free_block->prev = NULL;

    if (heap->free_list) {
        ((FreeBlock *)heap->free_list)->prev = free_block;
    }

    heap->free_list = free_block;
}

// Remove a block from the free list
static void remove_free_block(SdbHeap *heap, BlockHeader *block) {
    if (!block || !block->free) {
        return;
    }

    FreeBlock *free_block = (FreeBlock *)get_user_ptr(block);
    
    if (free_block->prev) {
        free_block->prev->next = free_block->next;
    } else {
        heap->free_list = free_block->next;
    }

    if (free_block->next) {
        free_block->next->prev = free_block->prev;
    }
}

// Merge adjacent free blocks
static BlockHeader *merge_blocks(SdbHeap *heap, BlockHeader *block) {
    if (!block || !block->free) {
        return block;
    }

    // Try to merge with the next block
    if (block->next && block->next->free) {
        remove_free_block(heap, block->next);
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Try to merge with the previous block
    if (block->prev && block->prev->free) {
        remove_free_block(heap, block);
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        return block->prev;
    }

    return block;
}

// Split a block if it's large enough
static void split_block(SdbHeap *heap, BlockHeader *block, size_t size) {
    // Only split if we have enough space for a new block plus some data
    size_t min_size = sizeof(BlockHeader) + MIN_ALLOC_SIZE;
    if (block->size < size + min_size) {
        return;
    }

    // Create new block at the end of the current block
    size_t remaining = block->size - size;
    BlockHeader *new_block = (BlockHeader *)((char *)block + size);
    new_block->size = remaining;
    new_block->free = true;
    new_block->next = block->next;
    new_block->prev = block;

    // Update the original block
    block->size = size;
    block->next = new_block;

    if (new_block->next) {
        new_block->next->prev = new_block;
    }

    // Add the new block to the free list
    insert_free_block(heap, new_block);
}

// Initialize the heap
SDB_API void sdb_heap_init(SdbHeap *heap) {
    heap->head = NULL;
    heap->free_list = NULL;
    heap->total_size = 0;
    heap->used_size = 0;
}

// Clean up and deallocate all memory
SDB_API void sdb_heap_fini(SdbHeap *heap) {
    if (!heap || !heap->head) {
        return;
    }

    BlockHeader *current = heap->head;
    BlockHeader *next;

    // Go through all blocks and unmap them if they came from mmap
    while (current) {
        next = current->next;
        size_t block_size = current->size;
        
        // Check if the block is page-aligned
        if (((uintptr_t)current % SDB_PAGE_SIZE) == 0 && block_size >= SDB_PAGE_SIZE) {
            munmap(current, block_size);
        }
        
        current = next;
    }

    heap->head = NULL;
    heap->free_list = NULL;
    heap->total_size = 0;
    heap->used_size = 0;
}

// Allocate memory
static void *sdb_heap_malloc(SdbHeap *heap, size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Align size to include header and ensure minimum allocation
    size_t aligned_size = ALIGN(size + sizeof(BlockHeader));
    if (aligned_size < sizeof(BlockHeader) + sizeof(FreeBlock)) {
        aligned_size = sizeof(BlockHeader) + sizeof(FreeBlock);
    }

    // First, try to find a free block that's big enough
    FreeBlock *free_block = heap->free_list;
    BlockHeader *best_fit = NULL;
    size_t best_size = SIZE_MAX;

    while (free_block) {
        BlockHeader *header = get_header(free_block);
        if (header->free && header->size >= aligned_size) {
            // Use best-fit strategy
            if (header->size < best_size) {
                best_fit = header;
                best_size = header->size;
                
                // If we find an exact match, use it immediately
                if (best_size == aligned_size) {
                    break;
                }
            }
        }
        free_block = free_block->next;
    }

    // If we found a suitable free block, use it
    if (best_fit) {
        remove_free_block(heap, best_fit);
        
        // Split the block if it's large enough
        if (best_fit->size >= aligned_size + sizeof(BlockHeader) + MIN_ALLOC_SIZE) {
            split_block(heap, best_fit, aligned_size);
        }
        
        best_fit->free = false;
        heap->used_size += best_fit->size;
        return get_user_ptr(best_fit);
    }

    // No suitable free block found, allocate new memory
    size_t alloc_size = aligned_size;
    if (alloc_size < SDB_PAGE_SIZE) {
        alloc_size = SDB_PAGE_SIZE;
    } else {
        // Round up to page size
        alloc_size = ALIGN(alloc_size);
        size_t pages = (alloc_size + SDB_PAGE_SIZE - 1) / SDB_PAGE_SIZE;
        alloc_size = pages * SDB_PAGE_SIZE;
    }

    BlockHeader *new_block = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, 
                                 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (new_block == MAP_FAILED) {
        return NULL;
    }

    // Initialize the new block
    new_block->size = alloc_size;
    new_block->free = false;
    new_block->next = NULL;
    new_block->prev = NULL;

    // Add to our linked list of blocks
    if (heap->head) {
        BlockHeader *current = heap->head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_block;
        new_block->prev = current;
    } else {
        heap->head = new_block;
    }

    heap->total_size += alloc_size;
    heap->used_size += aligned_size;

    // If we have extra space, create a free block
    if (alloc_size > aligned_size) {
        split_block(heap, new_block, aligned_size);
    }

    return get_user_ptr(new_block);
}

// Free memory
static void sdb_heap_free(SdbHeap *heap, void *ptr) {
    if (!ptr) {
        return;
    }

    BlockHeader *header = get_header(ptr);
    if (!header || header->free) {
        return; // Already freed or invalid pointer
    }

    header->free = true;
    heap->used_size -= header->size;

    // Add to free list
    insert_free_block(heap, header);

    // Merge with adjacent blocks if possible
    BlockHeader *merged = merge_blocks(heap, header);

    // If the block is large enough and page-aligned, return it to the OS
    if (merged->size >= SDB_PAGE_SIZE && ((uintptr_t)merged % SDB_PAGE_SIZE) == 0) {
        // Unlink the block from our structures
        if (merged->prev) {
            merged->prev->next = merged->next;
        } else {
            heap->head = merged->next;
        }
        
        if (merged->next) {
            merged->next->prev = merged->prev;
        }
        
        remove_free_block(heap, merged);
        
        // Return the memory to the OS
        size_t size = merged->size;
        munmap(merged, size);
        heap->total_size -= size;
    }
}

// Reallocate memory
SDB_API void *sdb_heap_realloc(SdbHeap *heap, void *ptr, size_t size) {
    // If ptr is NULL, equivalent to malloc
    if (!ptr) {
        return sdb_heap_malloc(heap, size);
    }

    // If size is 0, equivalent to free
    if (size == 0) {
        sdb_heap_free(heap, ptr);
        return NULL;
    }

    BlockHeader *header = get_header(ptr);
    if (!header) {
        return NULL; // Invalid pointer
    }

    size_t aligned_size = ALIGN(size + sizeof(BlockHeader));
    
    // If the current block is big enough, we can use it directly
    if (header->size >= aligned_size) {
        // We might be able to split this block
        split_block(heap, header, aligned_size);
        return ptr;
    }

    // Try to merge with the next block if it's free
    if (header->next && header->next->free) {
        remove_free_block(heap, header->next);
        size_t total_size = header->size + header->next->size;
        
        if (total_size >= aligned_size) {
            header->size = total_size;
            header->next = header->next->next;
            
            if (header->next) {
                header->next->prev = header;
            }
            
            // We might be able to split this block
            split_block(heap, header, aligned_size);
            return ptr;
        }
    }

    // Need to allocate a new block
    void *new_ptr = sdb_heap_malloc(heap, size);
    if (!new_ptr) {
        return NULL;
    }

    // Copy the data from the old block to the new one
    size_t copy_size = header->size - sizeof(BlockHeader);
    if (copy_size > size) {
        copy_size = size;
    }
    
    memcpy(new_ptr, ptr, copy_size);
    
    // Free the old block
    sdb_heap_free(heap, ptr);
    
    return new_ptr;
}

// Set up the global heap custom implementation
const SdbGlobalHeap sdb_gh_custom = {
    (SdbHeapRealloc)sdb_heap_realloc,
    (SdbHeapFini)sdb_heap_fini,
    &sdb_heap_custom_data
};

#endif
#endif