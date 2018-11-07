#ifndef R_JUDY_H
#define R_JUDY_H 1

#include <memory.h>
#include <string.h>
#include <r_types.h>

#ifdef linux
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define __USE_FILE_OFFSET64

#include <endian.h>
#else
#ifdef __BIG_ENDIAN__
#ifndef BYTE_ORDER
#define BYTE_ORDER 4321
#endif
#else
#ifndef BYTE_ORDER
#define BYTE_ORDER 1234
#endif
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif
#endif

typedef unsigned char uchar;
typedef unsigned int uint;
#define PRIuint "u"

#if defined(__LP64__) ||\
	defined(__x86_64__) ||\
	defined(__amd64__) ||\
	defined(_WIN64) ||\
	defined(__sparc64__) ||\
	defined(__arch64__) ||\
	defined(__powerpc64__) ||\
	defined(__s390x__)
//	defines for 64 bit

typedef unsigned long long judyvalue;
typedef unsigned long long JudySlot;
#define JUDY_key_mask (0x07)
#define JUDY_key_size 8
#define JUDY_slot_size 8
#define JUDY_span_bytes (3 * JUDY_key_size)
#define JUDY_span_equiv JUDY_2
#define JUDY_radix_equiv JUDY_8

#define PRIjudyvalue "llu"

#else
//	defines for 32 bit

typedef uint judyvalue;
typedef uint JudySlot;
#define JUDY_key_mask (0x03)
#define JUDY_key_size 4
#define JUDY_slot_size 4
#define JUDY_span_bytes (7 * JUDY_key_size)
#define JUDY_span_equiv JUDY_4
#define JUDY_radix_equiv JUDY_8

#define PRIjudyvalue "u"

#endif

#define JUDY_mask (~(JudySlot)0x07)

//	define the alignment factor for judy nodes and allocations
//	to enable this feature, set to 64

#define JUDY_cache_line 8 // minimum size is 8 bytes

#define JUDY_seg 65536

enum JUDY_types {
	JUDY_radix = 0, // inner and outer radix fan-out
	JUDY_1 = 1, // linear list nodes of designated count
	JUDY_2 = 2,
	JUDY_4 = 3,
	JUDY_8 = 4,
	JUDY_16 = 5,
	JUDY_32 = 6,
#ifdef ASKITIS
	JUDY_64 = 7
#else
	JUDY_span = 7 // up to 28 tail bytes of key contiguously stored
#endif
};


typedef struct {
	void *seg; // next used allocator
	uint next; // next available offset
} JudySeg;

typedef struct {
	JudySlot next; // judy object
	uint off; // offset within key
	int slot; // slot within object
} JudyStack;

typedef struct {
	JudySlot root[1]; // root of judy array
	void **reuse[8]; // reuse judy blocks
	JudySeg *seg; // current judy allocator
	uint level; // current height of stack
	uint max; // max height of stack
	uint depth; // number of Integers in a key, or zero for string keys
	JudyStack stack[1]; // current cursor
} Judy;

R_API void judy_close(Judy *judy);
R_API void *judy_alloc(Judy *judy, uint type);
R_API void *judy_data(Judy *judy, uint am);
R_API void *judy_clone(Judy *judy);
R_API void judy_free(Judy *judy, void *block, int type);
R_API uint judy_key(Judy *judy, uchar *buff, uint max);
R_API JudySlot *judy_slot(Judy *judy, uchar *buff, uint max);
R_API JudySlot *judy_promote(Judy *judy, JudySlot *next, int idx, judyvalue value, int keysize);
R_API void judy_radix(Judy *judy, JudySlot *radix, uchar *old, int start, int slot, int keysize, uchar key, uint depth);
R_API void judy_splitnode(Judy *judy, JudySlot *next, uint size, uint keysize, uint depth);
R_API JudySlot *judy_first(Judy *judy, JudySlot next, uint off, uint depth);
R_API JudySlot *judy_last(Judy *judy, JudySlot next, uint off, uint depth);
R_API JudySlot *judy_end(Judy *judy);
R_API JudySlot *judy_nxt(Judy *judy);
R_API JudySlot *judy_prv(Judy *judy);
R_API JudySlot *judy_del(Judy *judy);
R_API JudySlot *judy_strt(Judy *judy, uchar *buff, uint max);
R_API void judy_splitspan(Judy *judy, JudySlot *next, uchar *base);
R_API JudySlot *judy_cell(Judy *judy, uchar *buff, uint max);

#endif
