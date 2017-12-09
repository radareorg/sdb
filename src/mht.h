
#define MHTSZ 32
#define MHTNO 0

typedef ut64 mhti;

typedef struct {
	mhti k;
	mhti v;
	void *u;
#if 0
	// unaligned
	// on 32bits
	void *pad;
	// on 64bits
	void *pad;
#endif
} mhtkv;

// 4 + 4 + 4 = 12 .. missing 4 more
// 8 + 8 + 4 = 20 .. missing 16, what about 32 ?
// 8 + 8 + 8 = 24 .. still not there, missing 8
// 4 + 4 + 8 = 16 .. lgtm

typedef void (*mht_free)(void *);

typedef struct {
	void *table[MHTSZ];
	mht_free f;
} mht;

typedef mht SdbMini;


