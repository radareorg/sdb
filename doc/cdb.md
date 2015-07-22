Disk Hashtable
==============

CDB is the disk hashtable used to store the key-value pairs in a file.

This file aims to explain how the format.

[0..2048] - hash slots
[2048...] - array of:
	[ 4 byte ] - 1 byte for key length, 3 bytes for value length
	[ N byte ] - key string
	[ N byte ] - value string
[hslots]  - pointed out by the first 2048 table
	[ pointers to the keyvalue table ]
