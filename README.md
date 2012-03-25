SDB (simple database)
=====================
author: pancake

Description
-----------
sdb is a simple key/value database with disk storage.
mcsdbd is a memcache server with disk storage based on sdb.
sdbtypes is a vala library that implements several data
   structures on top of an sdb or memcache instance.

Rips
----
disk storage based on cdb code
memory hashtable based on wayland code
linked lists from r2 api

Changes
-------
I have slightly modified the cdb code to get smaller databases
and be memory leak free.

The sdb's cdb database format is 10% smaller than the original
one. This is because keylen and valuelen are encoded in 4 bytes
using two endian-safe unsigned short values.

In a test case, a 4.3MB cdb database takes only 3.9MB after this
file format change.

Backups
-------
To make a backup of a database to move it between different boxes use the textual format:

	$ sdb my.db | xz > my.xz
	$ du -hs my.*
	my.db        3.9M
	my.xz        5K

Using ascii+xz is the best option for storing compressed sdb databases.

	$ xz -9 my.db ; du -hs my.db.xz ; xz -d my.db
	my.db.xz     37K

To import the database:

	$ xz -d < my.xz | sdb my.db =
