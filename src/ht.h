#include "list.h"
#include "types.h"

/** ht **/
typedef struct r_ht_entry_t {
	RListIter *iter;
	ut32 hash;
	void *data;
} RHashTableEntry;

typedef struct r_ht_t {
	RList *list;
	RHashTableEntry *table;
	ut32 size;
	ut32 rehash;
	ut32 max_entries;
	ut32 size_index;
	ut32 entries;
	ut32 deleted_entries;
} RHashTable;

RHashTable* r_ht_new(void);
void r_ht_free(RHashTable *ht);
void r_ht_set(RHashTable *ht, ut32 hash, void *data);
RHashTableEntry* r_ht_search(RHashTable *ht, ut32 hash);
void *r_ht_lookup(RHashTable *ht, ut32 hash);
void r_ht_set(RHashTable *ht, ut32 hash, void *data);
int r_ht_insert(RHashTable *ht, ut32 hash, void *data, RListIter *iter);
void r_ht_remove(RHashTable *ht, ut32 hash);
void r_ht_remove_entry(RHashTable *ht, RHashTableEntry *entry);
