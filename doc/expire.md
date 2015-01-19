Expiration time
===============

Keys in memory contain an expiration time field that specifies how much time are they expected to be valid. Those keys are never synced to disk, so they should be used for temporal use. This can be useful in order to enable a pseudo "automatic garbage collection" that will free the expired keys when calling `sdb_sync()`.

Expire times are specified in seconds.

The disk storage doesn't stores the expiration field. It should be used only for static storage.

Only `sdb_get` and `sdb_sync` verifies this value in order to unset it from memory.

This is an usage example:

	Sdb *s = sdb_new0 ();
	sdb_set (s, "key", "val", 0);
	sdb_expire_set (s, "key", 10, 0);
	sleep (3);
	assert (sdb_const_get (s, "key", 0) != NULL);
	sleep (7);
	assert (sdb_const_get (s, "key", 0) == NULL);
	sdb_free (s);

The available APIs are:

	sdb_expire_set (Sdb *db, string *key, int seconds, ut32 cas);
	sdb_expire_get (Sdb *db, string *key, ut32 *cas);
