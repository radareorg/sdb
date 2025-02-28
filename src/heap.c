#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include "sdb/sdb.h"

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
#endif


// Align pointers and sizes to 8 bytes (suitable for 64-bit alignment)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SDB_PAGE_SIZE 131072  // 128 KB pages
#define MIN_SIZE ALIGN(sizeof(free_list) + META_SIZE)  // Minimum block size to hold free_list node + metadata

// Macros for page calculations
#define CEIL_DIV(x,y) (((x) + (y) - 1) / (y))
#define PAGES(size) CEIL_DIV((size), SDB_PAGE_SIZE)

// Structure for free list node (embedded in free block's payload)
typedef struct free_list {
	struct free_list *next;
	struct free_list *prev;
} free_list;

// Header and Footer definitions
typedef struct Header {
	int size;               // total size of this block (including header+footer+payload)
	bool free : 1;          // 1-bit flag: true if block is free
	bool has_prev : 1;      // true if a previous contiguous block exists
	bool has_next : 1;      // true if a next contiguous block exists
				// (Remaining bits of the byte/word are unused padding due to bit-fields)
} Header;

typedef struct Footer {
	int size;
	bool free : 1;
	// (Padding bits unused)
} Footer;

// Compute aligned metadata sizes
#define HEADER_SIZE ALIGN(sizeof(Header))
#define FOOTER_SIZE ALIGN(sizeof(Footer))
#define META_SIZE ALIGN(sizeof(Header) + sizeof(Footer))

// Global heap state structure
typedef struct sdb_heap_t {
	uint8_t *last_address;      // End address of the last allocated region (for contiguous mmap hint)
	free_list *free_list_start; // Head of the free list linked list
	size_t last_mapped_size;    // Size (in pages) of the last mmap request (doubling strategy)
} SdbHeap;

// Utility functions to get pointers to header, payload, and footer
static inline void *add_offset(void *header_ptr) {
	// From a Header pointer, get the address of the payload
	return (uint8_t*)header_ptr + HEADER_SIZE;
}
static inline void *remove_offset(void *payload_ptr) {
	// From a payload pointer, get the address of the Header
	return (uint8_t*)payload_ptr - HEADER_SIZE;
}
static inline Footer *getFooter(void *header_ptr) {
	Header *h = (Header*)header_ptr;
	return (Footer*)((uint8_t*)header_ptr + h->size - FOOTER_SIZE);
}

// Functions to set and get block attributes
static inline void setFreeFlag(void *header_ptr, bool is_free) {
	Header *h = (Header*)header_ptr;
	h->free = is_free;
	// Set the footer's free flag and size to match header
	Footer *f = getFooter(header_ptr);
	f->free = is_free;
	f->size = h->size;
}
static inline int getBlockSize(void *payload_ptr) {
	// Get total block size given a pointer to the payload
	Header *h = (Header*)remove_offset(payload_ptr);
	return h->size;
}

// Free list management functions
static void remove_from_free_list(SdbHeap *heap, void *header_ptr) {
	// Remove the free block with this header from the free list
	free_list *node = (free_list*)add_offset(header_ptr);
	free_list *prev = node->prev;
	free_list *next = node->next;
	if (prev) prev->next = next;
	else heap->free_list_start = next;      // Removing head
	if (next) next->prev = prev;
	// Mark block as used (not free) since it's no longer in free list
	setFreeFlag(header_ptr, false);
}
static void append_to_free_list(SdbHeap *heap, void *header_ptr) {
	// Add this free block (by header pointer) to the free list (LIFO insertion at head)
	Header *h = (Header*)header_ptr;
	h->free = true;
	Footer *f = getFooter(header_ptr);
	f->free = true;
	f->size = h->size;
	// Initialize a free_list node in the payload area
	free_list *node = (free_list*)add_offset(header_ptr);
	node->prev = NULL;
	node->next = heap->free_list_start;
	if (heap->free_list_start) {
		heap->free_list_start->prev = node;
	}
	heap->free_list_start = node;
}
static free_list *find_free_block(SdbHeap *heap, int size) {
	// First-fit search for a free block with at least 'size' bytes
	free_list *current = heap->free_list_start;
	while (current) {
		Header *h = (Header*)remove_offset(current);
		if (h->size >= size) {
			return current; // found a large enough free block
		}
		current = current->next;
	}
	return NULL; // no suitable block found
}

// Split a block into an allocated part and a free remainder
static void split_block(SdbHeap *heap, void *header_ptr, int total_size, int alloc_size) {
	// Only split if the remainder is large enough to form a new free block
	int remainder_size = total_size - alloc_size;
	if (remainder_size < (int)MIN_SIZE) {
		return; // Not enough space to create another block; skip splitting
	}
	Header *orig_header = (Header*)header_ptr;
	// Create the new free block at the end of the allocated portion
	uint8_t *new_block_addr = (uint8_t*)header_ptr + alloc_size;
	Header *new_header = (Header*)new_block_addr;
	// Set up the allocated block (orig_header) metadata
	orig_header->size = alloc_size;
	orig_header->has_next = true;
	// Set up the new free block's header
	new_header->size = remainder_size;
	new_header->free = true;
	new_header->has_prev = true;
	new_header->has_next = orig_header->has_next; // inherit whether a block existed beyond the original
						      // Update orig_header's next flag to true (it now has a neighbor)
						      // (orig_header->has_next is already true from above)
						      // Update the footer of the new free block
	Footer *new_footer = getFooter(new_header);
	new_footer->size = remainder_size;
	new_footer->free = true;
	// Link the new free block into the free list
	append_to_free_list(heap, new_header);
	// Update the *next* block (if it exists) to have a prev neighbor
	if (new_header->has_next) {
		// The block after the new free block (if any) should now have a prev
		Header *next_header = (Header*)((uint8_t*)new_block_addr + new_header->size);
		next_header->has_prev = true;
	}
}

// Initialize and finalize heap (for completeness)
void sdb_heap_init(SdbHeap *heap) {
	heap->last_address = NULL;
	heap->free_list_start = NULL;
	heap->last_mapped_size = 1;  // start with 1 page on first allocation
}
void sdb_heap_fini(SdbHeap *heap) {
	// Free all free blocks (release to OS)
	free_list *current = heap->free_list_start;
	while (current) {
		Header *h = (Header*)remove_offset(current);
		// Unmap free block regions (only if they were allocated via mmap)
		// Here we simply iterate and unmap all free blocks.
		munmap(h, h->size);
		current = current->next;
	}
	heap->free_list_start = NULL;
}

// Allocate memory
void *sdb_heap_malloc(SdbHeap *heap, int size) {
	if (size <= 0) {
		return NULL;
	}
	// Calculate required total block size (include header+footer, aligned)
	int required_size = ALIGN(size + META_SIZE);
	if (required_size < (int)MIN_SIZE) {
		required_size = MIN_SIZE; // ensure minimum block size
	}
	// Try to find a free block in the existing heap
	free_list *free_node = find_free_block(heap, required_size);
	if (free_node) {
		// Reuse this free block
		Header *header = (Header*)remove_offset(free_node);
		remove_from_free_list(heap, header);
		// Mark as used and possibly split if it's larger than needed
		setFreeFlag(header, false);
		split_block(heap, header, header->size, required_size);
		// Return pointer to payload
		return add_offset(header);
	}
	// No suitable free block found; map a new region from OS
	size_t pages_needed = PAGES(required_size);
	size_t pages_to_map = heap->last_mapped_size;
	if (pages_needed > pages_to_map) {
		pages_to_map = pages_needed;
	}
	size_t bytes = pages_to_map * SDB_PAGE_SIZE;
	// Double the last_mapped_size for next time (to exponential grow)
	heap->last_mapped_size = (heap->last_mapped_size < pages_needed) 
		? heap->last_mapped_size * 2 
		: heap->last_mapped_size * 2;
	void *new_region = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_region == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	// Set up the new region as a single allocated block for now
	Header *header = (Header*)new_region;
	header->size = (int)bytes;
	header->free = false;
	header->has_prev = false;
	header->has_next = false;
	// If this new region is contiguous with the last allocated region, link them
	if (heap->last_address != NULL && new_region == heap->last_address) {
		// The new region starts exactly where the previous one ended
		header->has_prev = true;
		// Update previous block's has_next flag
		// Find the previous block's header using the footer just before this region
		Footer *prev_footer = (Footer*)((uint8_t*)new_region - FOOTER_SIZE);
		Header *prev_header = (Header*)((uint8_t*)new_region - prev_footer->size);
		prev_header->has_next = true;
	}
	// Possibly split off the portion needed by the user from this new region
	// We treat the portion to return as allocated and the remainder as free.
	split_block(heap, header, (int)bytes, required_size);
	// Update the last_address to the end of this region for future contiguous allocations
	heap->last_address = (uint8_t*)new_region + bytes;
	// Return pointer to payload of allocated block
	return add_offset(header);
}

// Free memory
void sdb_heap_free(SdbHeap *heap, void *ptr) {
	if (!ptr) return;
	Header *header = (Header*)remove_offset(ptr);
	if (header->free) {
		// Block is already free – likely a double free – ignore to avoid corruption
		return;
	}
	int size = header->size;
	uintptr_t addr = (uintptr_t)header;
	// If the block is a whole number of pages and page-aligned, free it back to OS
	if ((size % SDB_PAGE_SIZE) == 0 && (addr % SDB_PAGE_SIZE) == 0) {
		// Unlink neighbor relationships before unmapping
		if (header->has_prev) {
			// Previous block exists: set its has_next to false
			Footer *prev_footer = (Footer*)((uint8_t*)header - FOOTER_SIZE);
			Header *prev_header = (Header*)((uint8_t*)header - prev_footer->size);
			prev_header->has_next = false;
		}
		if (header->has_next) {
			// Next block exists: set its has_prev to false
			Header *next_header = (Header*)((uint8_t*)header + header->size);
			next_header->has_prev = false;
		}
		// Update heap->last_address if this was the last region
		if (heap->last_address == (uint8_t*)header + header->size) {
			heap->last_address = (uint8_t*)header; // move last_address backward
		}
		// Return the memory to the OS
		munmap(header, header->size);
		return;
	}
	// Otherwise, mark the block free and add to free list
	append_to_free_list(heap, header);
	// Coalesce with adjacent free blocks if possible
	// Coalesce backward
	Header *current_header = header;
	//bool merged_prev = false;
	if (current_header->has_prev) {
		// Check the previous block's footer to see if the previous block is free
		Footer *prev_footer = (Footer*)((uint8_t*)current_header - FOOTER_SIZE);
		if (prev_footer->free) {
			// Previous block is free: merge with it
			Header *prev_header = (Header*)((uint8_t*)current_header - prev_footer->size);
			// Remove the current block from free list (prev is already free and in list)
			remove_from_free_list(heap, current_header);
			// Combine sizes
			prev_header->size += current_header->size;
			// Use current block's footer as the new combined footer
			Footer *curr_footer = getFooter(current_header);
			// Now prev_header's block spans both, so update its footer (which is curr_footer's position)
			Footer *new_footer = curr_footer;
			new_footer->size = prev_header->size;
			new_footer->free = true;
			// Update flags: prev_header is still free, has_prev stays same, has_next should reflect current's has_next
			bool had_next = current_header->has_next;
			prev_header->has_next = had_next;
			current_header = prev_header; // current_header now points to the combined block's header (prev block)
			// merged_prev = true;
		}
	}
	// Coalesce forward
	if (current_header->has_next) {
		Header *next_header = (Header*)((uint8_t*)current_header + current_header->size);
		if (next_header->free) {
			// Next block is free: merge with it
			remove_from_free_list(heap, next_header);
			// Combine sizes
			current_header->size += next_header->size;
			// Update footer at end of combined block
			Footer *end_footer = getFooter(next_header);
			// end_footer currently is next_header's footer
			end_footer->size = current_header->size;
			end_footer->free = true;
			// Update flags for combined block
			current_header->has_next = next_header->has_next;
		}
	}
	// After coalescing, current_header is the start of the free region
	// If the resulting free block contains whole pages and is page-aligned, release those pages
	Header *free_block = current_header;
	size = free_block->size;
	addr = (uintptr_t)free_block;
	if (size >= SDB_PAGE_SIZE && (addr % SDB_PAGE_SIZE) == 0) {
		// Calculate how many whole pages can be freed from this block
		int whole_pages = (size / SDB_PAGE_SIZE);
		int bytes_to_free = whole_pages * SDB_PAGE_SIZE;
		// Split off the page-aligned chunk to free
		// We reuse split_block to carve out 'bytes_to_free' from the beginning of this free block
		remove_from_free_list(heap, free_block); // remove combined block from list temporarily
		setFreeFlag(free_block, false); // mark as used while we carve it (avoid confusion)
		split_block(heap, free_block, size, bytes_to_free);
		// Now the first part of free_block is of size bytes_to_free and marked used (not in free list).
		// Unmap that first part
		munmap(free_block, bytes_to_free);
		// The second part (if any) is already in the free list from split_block.
		// Update heap->last_address if needed
		if (heap->last_address == (uint8_t*)free_block + size) {
			heap->last_address -= bytes_to_free;
		}
		// If the entire block was freed (no remainder), do nothing else.
		// If there is a remainder free block, it’s already in the free list.
	}
}

// Reallocate memory (resize block)
void *sdb_heap_realloc(SdbHeap *heap, void *ptr, int new_size) {
	if (!ptr) {
		// Equivalent to malloc
		return sdb_heap_malloc(heap, new_size);
	}
	if (new_size <= 0) {
		// Equivalent to free (with special case: allocate minimum object of size 1 as per code)
		sdb_heap_free(heap, ptr);
		return sdb_heap_malloc(heap, 1);
	}
	Header *current_header = (Header*)remove_offset(ptr);
	int old_total_size = current_header->size;
	int old_payload_size = old_total_size - (int)META_SIZE;
	int required_size = ALIGN(new_size + META_SIZE);
	if (required_size < (int)MIN_SIZE) {
		required_size = MIN_SIZE;
	}
	// If the current block is already large enough to accommodate new_size, reuse it
	if (required_size <= old_total_size) {
		// Optionally, we could shrink the block if the new request is much smaller, 
		// but for now we simply return the same pointer (no move).
		return ptr;
	}
	// Try to expand into the next block if it's free
	if (current_header->has_next) {
		Header *next_header = (Header*)((uint8_t*)current_header + current_header->size);
		if (next_header->free) {
			int combined_size = current_header->size + next_header->size;
			if (combined_size >= required_size) {
				// We can merge with next block to satisfy the request
				// Remove next block from free list and merge
				remove_from_free_list(heap, next_header);
				current_header->size = combined_size;
				current_header->has_next = next_header->has_next;
				// Update footer to combined block
				Footer *combined_footer = getFooter(current_header);
				combined_footer->size = current_header->size;
				combined_footer->free = false; // block will be in-use after realloc
							       // If there's still extra space beyond required_size, split the excess into a free block
				split_block(heap, current_header, current_header->size, required_size);
				return ptr;
			}
		}
	}
	// Otherwise, allocate a new block and copy the data
	void *new_ptr = sdb_heap_malloc(heap, new_size);
	if (!new_ptr) {
		return NULL; // allocation failed
	}
	// Copy old data to new block (up to the smaller of old data size or new size)
	int bytes_to_copy = old_payload_size;
	if (bytes_to_copy > new_size) {
		bytes_to_copy = new_size;
	}
	memcpy(new_ptr, ptr, bytes_to_copy);
	// Free the old block
	sdb_heap_free(heap, ptr);
	return new_ptr;
}
