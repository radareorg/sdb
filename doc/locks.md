Locks
=====

Locks are used to limit the use of a sdb disk database by only one instance of Sdb.

This avoids the problem of accessing to the same database by more than one process.

Bear in mind that Sdb stores everything in memory until `sdb_sync` is called, so locks are useful to avoid other processes get outdated or useless information from the unsynced database.

The locks permit to check and wait for release operations, so your application must use them if there is more than one process accessing the database to store information.

Lockfiles are just a file named '<dbname>.lock' containing the process id (PID). This is later checked by the user program to detect stale locks.

this is a sample program that locks a database:

	#define DBNAME "test.sdb"
	retry:
	Sdb *s = sdb_new (NULL, DBNAME, 1);
	if (!s) {
		const char *lf = sdb_lockfile (DBNAME);
		sdb_lock_wait (lf);
		sdb_unlock (lf); // race condition
		goto retry;
	}
	sdb_free (s);


Another solution would be to use '2' as 3rd parameter of `sdb_new` to make it wait until the lock is stale.

	#define DBNAME "test.sdb"
	retry:
	Sdb *s = sdb_new (NULL, DBNAME, 2);
	...
	sdb_free (s);

