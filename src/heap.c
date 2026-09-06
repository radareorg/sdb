// https://github.com/YeonwooSung/MemoryAllocation

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "sdb/sdb.h"
#include "sdb/heap.h"

static SdbGlobalHeap Gheap = { NULL, NULL, NULL };

SDB_API SdbGlobalHeap *sdb_gh(void) {
	return &Gheap;
}

SDB_API char *sdb_strdup(const char *s) {
	size_t sl = strlen (s) + 1;
	char *p = (char *)sdb_gh_malloc (sl);
	if (p) {
		memcpy (p, s, sl);
	}
	return p;
}

#if USE_MMAN
// #include <sys/mman.h>

#if !defined(MAP_ANONYMOUS)
#if __MACH__
  #define MAP_ANONYMOUS 0x1000
#else
  #define MAP_ANONYMOUS 0x20
#endif
#endif

#if __SDB_WINDOWS__
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

// Size 16
typedef struct free_list {
	struct free_list *next;
	struct free_list *prev;
} free_list;

typedef struct sdb_heap_t {
	// Globals
	ut8 *last_address;
	free_list *free_list_start;
	// To reduce number of mmap calls.
	size_t last_mapped_size; // pages, starts at 1.
} SdbHeap;

SDB_API void sdb_heap_fini(SdbHeap *heap);
SDB_API void *sdb_heap_realloc(SdbHeap *heap, void *ptr, size_t size);

static void sdb_heap_fini_cb(void *data) {
	sdb_heap_fini ((SdbHeap *)data);
}

static void *sdb_heap_realloc_cb(void *data, void *ptr, size_t size) {
	return sdb_heap_realloc ((SdbHeap *)data, ptr, size);
}

static SdbHeap sdb_gh_custom_data = { NULL, NULL, 1};
const SdbGlobalHeap sdb_gh_custom = {
	sdb_heap_realloc_cb,
	sdb_heap_fini_cb,
	&sdb_gh_custom_data
};

#define USED false
#define FREE true

typedef struct Header {
	int size;
	bool free : 1;
	bool has_prev : 1;
	bool has_next : 1;
} Header;

// Size field is not necessary in used blocks.
typedef struct Footer {
	int size;
	bool free : 1;
} Footer;


#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SDB_PAGE_SIZE 131072 // 96096*2 //sysconf(_SC_PAGESIZE)
#define PAGES(size) (((size) + SDB_PAGE_SIZE - 1) / SDB_PAGE_SIZE)
#define MIN_SIZE (ALIGN(sizeof(free_list) + META_SIZE))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MAX_BLOCK_SIZE ((size_t)INT_MAX)
#define MAX_MAPPED_PAGES (MAX_BLOCK_SIZE / SDB_PAGE_SIZE)

// Meta sizes.
#define META_SIZE ALIGN(sizeof(Header) + sizeof(Footer))
#define HEADER_SIZE ALIGN(sizeof(Header))
#define FOOTER_SIZE ALIGN(sizeof(Footer))

// Get pointer to the payload (passing the pointer to the header).
static void *add_offset(Header *ptr) {
	return (void *)((const ut8*)ptr + HEADER_SIZE);
}

// Get pointer to the header (passing pointer to the payload).
static Header *remove_offset(void *ptr) {
	return (Header *)((const ut8*)ptr - HEADER_SIZE);
}

static Footer *get_footer(const Header *header_ptr) {
	return (Footer *)((const ut8*)header_ptr + header_ptr->size - FOOTER_SIZE);
}

static inline void write_footer(Header *ptr) {
	Footer *footer = get_footer (ptr);
	footer->size = ptr->size;
	footer->free = ptr->free;
}

static Footer *get_prev_footer(const Header *header_ptr) {
	if (!header_ptr->has_prev) {
		return NULL;
	}
	return (Footer *)((const ut8*)header_ptr - FOOTER_SIZE);
}

static Header *get_prev_header(const Header *header_ptr) {
	Footer *prev_footer = get_prev_footer(header_ptr);
	if (!prev_footer) {
		return NULL;
	}
	return (Header *)((const ut8*)header_ptr - prev_footer->size);
}

static Header *get_next_header(const Header *header_ptr) {
	if (!header_ptr->has_next) {
		return NULL;
	}
	return (Header *)((const ut8*)header_ptr + header_ptr->size);
}

static void setFree(Header *ptr, int val) {
	ptr->free = val;
	write_footer (ptr);
}

// Set size in the header.
static inline void setSizeHeader(Header *ptr, int size) {
	ptr->size = size;
}

#if 0
// Set size in the header.
static inline void setSizeFooter(void *ptr, int size) {
	((Footer *)getFooter(ptr))->size = size;
}
#endif

// Get size of the free list item.
static inline int getSize(void *ptr) {
	return remove_offset (ptr)->size;
}

static bool size_to_block_size(size_t size, int *required_size) {
	if (size == 0 || size > MAX_BLOCK_SIZE - META_SIZE) {
		return false;
	}
	size_t total = ALIGN (size + META_SIZE);
	if (total < MIN_SIZE) {
		total = MIN_SIZE;
	}
	if (total > MAX_BLOCK_SIZE) {
		return false;
	}
	*required_size = (int)total;
	return true;
}

static size_t clamp_mapped_pages(size_t mapped_pages) {
	if (mapped_pages > MAX_MAPPED_PAGES) {
		return MAX_MAPPED_PAGES;
	}
	return mapped_pages;
}

static size_t grow_mapped_pages(size_t mapped_pages) {
	if (mapped_pages >= (MAX_MAPPED_PAGES / 2)) {
		return MAX_MAPPED_PAGES;
	}
	return mapped_pages * 2;
}

static void remove_from_free_list(SdbHeap *heap, Header *block) {
	setFree (block, USED);

	free_list *free_block = (free_list *)add_offset(block);
	free_list *next = free_block->next;
	free_list *prev = free_block->prev;
	if (!prev) {
		if (!next) {
			// free_block is the only block in the free list.
			heap->free_list_start = NULL;
		} else {
			// Remove first element in the free list.
			heap->free_list_start = next;
			next->prev = NULL;
		}
	} else {
		if (!next) {
			// Remove last element of the free list.
			prev->next = NULL;
		} else {
			// Remove element in the middle.
			prev->next = next;
			next->prev = prev;
		}
	}
}

static void append_to_free_list(SdbHeap *heap, Header *ptr) {
	setFree (ptr, FREE);

	free_list eew = {};
	free_list *new_ptr = (free_list *)add_offset (ptr);
	*new_ptr = eew;

	if (heap->free_list_start) {
		// Insert in the beginning.
		new_ptr->next = heap->free_list_start;
		new_ptr->prev = NULL;
		heap->free_list_start->prev = new_ptr;
		heap->free_list_start = new_ptr;
	} else {
		// No elements in the free list
		heap->free_list_start = new_ptr;
		new_ptr->prev = NULL;
		new_ptr->next = NULL;
	}
}

// Find a free block that is large enough to store 'size' bytes.
// Returns NULL if not found.
static free_list *find_free_block(SdbHeap *heap, int size) {
	free_list *current = heap->free_list_start;
	while (current) {
		if (getSize (current) >= size) {
			// Return a pointer to the free block.
			return current;
		}
		current = current->next;
	}
	return NULL;
}

// Split memory into multiple blocks after some part of it was requested
// (requested + the rest).
static void split(SdbHeap *heap, Header *start_header, int total, int requested) {
	Header *new_block_header = (Header *)((ut8*)start_header + requested);
	int block_size = total - requested;

	// Size that was left after allocating memory.
	// Needs to be large enough to store another block (min size is needed in order
	// to store free list element there after it is freed).
	if (block_size < (int)MIN_SIZE) {
		// Not enough size to split.
		return;
	}
	// Change size of the prev (recently allocated) block.
	bool had_next = start_header->has_next;
	setSizeHeader(start_header, requested);
	start_header->has_next = true;
	write_footer (start_header);

	// Add a header for newly created block (right block).
	Header header = {block_size, FREE, true, had_next};
	*new_block_header = header;
	write_footer (new_block_header);
	// Ensure the next block (if present) points back to the new block.
	Header *next_header = get_next_header(new_block_header);
	if (next_header) {
		next_header->has_prev = true;
	}
	append_to_free_list (heap, new_block_header);
}

static void *sdb_heap_malloc(SdbHeap *heap, size_t size) {
	int required_size = 0;
	if (!size_to_block_size (size, &required_size)) {
		return NULL;
	}
	// Size of the block can't be smaller than MIN_SIZE, as we need to store
	// free list in the body + header and footer on each side respectively.
	// Try to find a block big enough in already allocated memory.
	free_list *free_block = find_free_block (heap, required_size);

	if (free_block) {
		// Header ptr
		Header *address = remove_offset (free_block);
		// Mark block as used.
		setFree (address, USED);
		// Split the block into two, where the second is free.
		split (heap, address, address->size, required_size);
		remove_from_free_list (heap, address);
		return add_offset (address);
	}

	// No free block was found. Allocate size requested + header (in full pages).
	// Each next allocation will be doubled in size from the previous one
	// (to decrease the number of mmap sys calls we make).
	size_t required_pages = PAGES ((size_t)required_size);
	if (required_pages > MAX_MAPPED_PAGES) {
		return NULL;
	}
	size_t mapped_pages = required_pages > heap->last_mapped_size
		? required_pages
		: heap->last_mapped_size;
	mapped_pages = clamp_mapped_pages (mapped_pages);
	size_t bytes = mapped_pages * SDB_PAGE_SIZE;
	int block_size = (int)bytes;
	heap->last_mapped_size = grow_mapped_pages (mapped_pages);

	// last_address my not be returned by mmap, but makes it more efficient if it happens.
	void *new_region = mmap (NULL, bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (new_region == MAP_FAILED) {
		perror ("mmap");
		return NULL;
	}
	// Create a header/footer for new block.
	Header header = {block_size, USED, false, false};
	Header *header_ptr = (Header *)new_region;
	*header_ptr = header;
	write_footer (header_ptr);

	if (new_region == heap->last_address && heap->last_address != 0) {
		// if we got a block of memory after the last block, as we requested.
		header_ptr->has_prev = true;
		// change has_next of the prev block
		Footer *prev_footer = get_prev_footer (header_ptr);
		Header *prev_header = get_prev_header (header_ptr);
		if (prev_footer && prev_header) {
			prev_header->has_next = true;
		}
	}
	// Split new region.
	split (heap, header_ptr, block_size, required_size);
	// Update last_address for the next allocation.
	heap->last_address = (ut8 *)new_region + block_size;
	// Return address behind the header (i.e. header is hidden).
	return add_offset (header_ptr);
}

static bool can_merge(Header *h1, Header *h2) {
	return (size_t)h1->size + (size_t)h2->size <= (size_t)INT_MAX;
}

static Header *coalesce(SdbHeap *heap, Header *current_header) {
	Header *merged_header = current_header;

	if (merged_header->has_prev) {
		Footer *prev_footer = get_prev_footer (merged_header);
		Header *prev_header = get_prev_header (merged_header);
		if (prev_footer && prev_footer->free && prev_header) {
			if (!can_merge (prev_header, merged_header)) {
				fprintf(stderr, "sdb_heap: critical overflow during prev merge\n");
			} else {
				remove_from_free_list (heap, merged_header);
				prev_header->size += merged_header->size;
				prev_header->has_next = merged_header->has_next;
				merged_header = prev_header;
			}
		}
	}

	if (merged_header->has_next) {
		Header *next_header = get_next_header (merged_header);
		if (next_header && next_header->free) {
			if (!can_merge (merged_header, next_header)) {
				fprintf(stderr, "sdb_heap: critical overflow during next merge\n");
			} else {
				remove_from_free_list (heap, next_header);
				merged_header->size += next_header->size;
				merged_header->has_next = next_header->has_next;
				Header *after_next = get_next_header (merged_header);
				if (after_next) {
					after_next->has_prev = true;
				}
			}
		}
	}

	setFree (merged_header, FREE);
	return merged_header;
}

static int unmap(SdbHeap *heap, Header *start_header, int size) {
	remove_from_free_list (heap, start_header);
	// Reset has_next, has_prev of neighbours.
	Header *prev_header = get_prev_header (start_header);
	if (prev_header) {
		prev_header->has_next = false;
	}
	Header *next_header = get_next_header (start_header);
	if (next_header) {
		next_header->has_prev = false;
	}

	// If this is the last block we've allocated using mmap, need to change last_address.
	if ((ut8 *)heap->last_address == (ut8 *)start_header + size) {
		// Keep the mmap tail hint only when a mapped predecessor still exists.
		heap->last_address = prev_header ? (ut8 *)start_header : NULL;
		if (!heap->last_address) {
			heap->last_mapped_size = 1;
		}
	}
	return munmap ((void *)start_header, (size_t)size);
}

static void sdb_heap_free(SdbHeap *heap, void *ptr) {
	if (!ptr) {
		return;
	}
	Header *start_header = remove_offset (ptr);

	// Check if it has already been freed.
	// Does not handle case when start_address passed was never allocated.
	if (start_header->free) {
		return;
	}

	append_to_free_list (heap, start_header);
	start_header = coalesce (heap, start_header);
	int size = start_header->size;
	uintptr_t addr = (uintptr_t)start_header;
	if (size >= SDB_PAGE_SIZE && (addr % SDB_PAGE_SIZE) == 0) {
		int unmap_size = (size / SDB_PAGE_SIZE) * SDB_PAGE_SIZE;
		if (unmap_size == size || size - unmap_size >= (int)MIN_SIZE) {
			if (unmap_size != size) {
				split (heap, start_header, size, unmap_size);
			}
			unmap (heap, start_header, unmap_size);
		}
	}
}

SDB_API void sdb_heap_init(SdbHeap *heap) {
	heap->last_address = NULL;
	heap->free_list_start = NULL;
	heap->last_mapped_size = 1;
}

SDB_API void sdb_heap_fini(SdbHeap *heap) {
#if 1
	free_list *current = heap->free_list_start;
	while (current) {
		free_list *next = current->next;
		sdb_heap_free (heap, current);
		current = next;
	}
#endif
}

SDB_API void *sdb_heap_realloc(SdbHeap *heap, void *ptr, size_t size) {
	// If ptr is NULL, realloc() is identical to a call to malloc() for size bytes.
	if (!ptr) {
		return sdb_heap_malloc (heap, size);
	}
	// If size is zero and ptr is not NULL, a new, minimum sized object (MIN_SIZE) is
	// treated as free().
	if (size == 0 && ptr) {
		sdb_heap_free (heap, ptr);
		return NULL;
	}

	int required_size = 0;
	if (!size_to_block_size (size, &required_size)) {
		return NULL;
	}
	// If there is enough space, expand the block.
	Header *current_header = remove_offset (ptr);
	int current_size = current_header->size;
	int payload_size = current_size - HEADER_SIZE - FOOTER_SIZE;

	// if user requests to shorten the block.
	if (size <= (size_t)payload_size) {
		return ptr;
	}
	Footer *current_footer = get_footer (current_header);
	// Next block exists and is free.
	Header *next_header = get_next_header (current_header);
	if (next_header && next_header->free && can_merge (current_header, next_header)) {
		int available_size = current_header->size + next_header->size;
		// Size is enough.
		if (available_size >= required_size) {
			remove_from_free_list (heap, next_header);
			// Add size of next block to the size of current block.
			current_header->size += next_header->size;
			current_header->has_next = next_header->has_next;
			Header *after_next = get_next_header (current_header);
			if (after_next) {
				after_next->has_prev = true;
			}
			current_footer = get_footer (current_header);
			current_footer->size = current_header->size;
			current_footer->free = USED;

			// split if possible.
			split (heap, current_header, available_size, required_size);
			setFree (current_header, USED);
			return ptr;
		}
	}

	// Not enough room to enlarge -> allocate new region.
	void *new_ptr = sdb_heap_malloc (heap, size);
	if (!new_ptr) {
		return NULL;
	}
	size_t copy_size = (size_t)payload_size < size ? (size_t)payload_size : size;
	memcpy (new_ptr, ptr, copy_size);

	// Free old location.
	sdb_heap_free (heap, ptr);
	return new_ptr;
}

#endif
#endif
