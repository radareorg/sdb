Backups
=======

Usually we want to keep old snapshots of sdb disk databases copied somewhere else as backups. For backups, taking as minimum disk space as possible is an important point.

In order to achieve this we tested the binary and textual database dumps. The results are pretty clear: To achieve the maximum compression for your databases use lzma over the textual dump.

Disk hashtables (based on cdb) are binary files which contain an mmaped image of a static hashtable, they are read only and optimized for such usage. So they contain lot of paddings and unused bytes.

For this reason it is easier to compress textual representations of the key=value pairs in plain text. LZMA have the best compression ratios for plain text.

This is a simple example, dumping the database into a xzipped text dump and comparing it to the original disk hashtable size:

	$ sdb my.db | xz > my.xz
	$ du -hs my.*
	my.db        3.9M
	my.xz        5K

Note that in the first line we are dumping the database and redirecting the output to a pipe where xz awaits for data to create the compressed backup.

Here there are some more tests with gzip, xz, lzma on the binary database and the textual dump:

	$ gzip < my.db | wc -c
	  110768
	$ xz -9 < my.db | wc -c
	  37480
	$ sdb my.db | xz -9 | wc -c
	  5620
	$ sdb my.db | lzma -9 | wc -c
	  5575

In order to recreate the database we must run something like this:

	$ xz -d < my.xz | sdb my.db =

The '=' argument means that the database will be created with the key=value pairs coming from stdin. This is faster than using '-' because that last will use an in-memory cache and then sync it to disk instead of just syncing to disk.
