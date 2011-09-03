#include "list.h"
#include "types.h"

/** ht **/
typedef struct r_ht_entry_t {
	SdbListIter *iter;
	ut32 hash;
	void *data;
} SdbHashEntry;

typedef struct r_ht_t {
	SdbList *list;
	SdbHashEntry *table;
	ut32 size;
	ut32 rehash;
	ut32 max_entries;
	ut32 size_index;
	ut32 entries;
	ut32 deleted_entries;
} SdbHash;

SdbHash* r_ht_new(void);
void r_ht_free(SdbHash *ht);
void r_ht_set(SdbHash *ht, ut32 hash, void *data);
SdbHashEntry* r_ht_search(SdbHash *ht, ut32 hash);
void *r_ht_lookup(SdbHash *ht, ut32 hash);
void r_ht_set(SdbHash *ht, ut32 hash, void *data);
int r_ht_insert(SdbHash *ht, ut32 hash, void *data, SdbListIter *iter);
void r_ht_remove(SdbHash *ht, ut32 hash);
void r_ht_remove_entry(SdbHash *ht, SdbHashEntry *entry);
