#include <leveldb/c.h>
#include <sdb.h>

int main() {
	struct leveldb_t *db;
	int i;
	db = leveldb_open (NULL, NULL, NULL);
	struct leveldb_writebatch_t* batch = leveldb_writebatch_create();
	for (i=0;i<100000; i++) {

		const char *key = sdb_fmt (0, "foo%d", i);
		leveldb_writebatch_put( batch, key, strlen (key), "bar", 3);
		//leveldb_put (db, 
	}
	leveldb_write(db, NULL, batch, NULL);
	leveldb_free (db);
}
