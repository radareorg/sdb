Backups
=======

To make a backup of a database to move it between different boxes use the textual format:

	$ sdb my.db | xz > my.xz
	$ du -hs my.*
	my.db        3.9M
	my.xz        5K

Using ascii+xz is the best option for storing compressed sdb databases:

	$ gzip < my.db | wc -c
	  110768
	$ xz -9 < my.db | wc -c
	  37480
	$ sdb my.db | xz -9 | wc -c
	  5620
	$ sdb my.db | lzma -9 | wc -c
	  5575

To import the database:

	$ xz -d < my.xz | sdb my.db =

