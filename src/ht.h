#include "list.h"
#include "types.h"

/** ht **/
typedef struct r_ht_entry_t {
	RListIter *iter;
	ut32 hash;
	void *data;
} RHashEntry;

typedef struct r_ht_t {
	RList *list;
	RHashEntry *table;
	ut32 size;
	ut32 rehash;
	ut32 max_entries;
	ut32 size_index;
	ut32 entries;
	ut32 deleted_entries;
} RHash;

RHash* r_ht_new(void);
void r_ht_free(RHash *ht);
void r_ht_set(RHash *ht, ut32 hash, void *data);
RHashEntry* r_ht_search(RHash *ht, ut32 hash);
void *r_ht_lookup(RHash *ht, ut32 hash);
void r_ht_set(RHash *ht, ut32 hash, void *data);
int r_ht_insert(RHash *ht, ut32 hash, void *data, RListIter *iter);
void r_ht_remove(RHash *ht, ut32 hash);
void r_ht_remove_entry(RHash *ht, RHashEntry *entry);
