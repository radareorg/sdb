Types
=====

Supported basic types are the following:

String
------

In Sdb, everything is a string, the key and the value. In some situations those strings can contain special characters, or they can even wanted to be filtered to reduce it in size.

	$ sdb - 'a=Hello' 'a'
	Hello

Binary
------
When we want to store special characters in strings (newline, comma, ...) or just store a binary blob as a value in a key; we will have to encode it using the `sdb_encode` and `sdb_decode` functions which preform a base64 transformation.

	$ sdb - '%a=Hello' 'a' '%a'
	SGVsbG8=
	Hello

Numbers
-------
Numbers are 64bit unsigned integers, negative values are handled as strings, so they would cast to 0.

	$ sdb - 'a=-3' '+a=1' 'a'
	1

64bit numbers can be used to store pointers to structures in memory.

The default number base is 16 (hexadecimal), but it inherits the one in the previous value if available.

	$ sdb - '+a=1' 'a'
	0x1
	$ sdb - 'a=0' '+a=1' 'a'
	1

Boolean
-------
true/false strings are handled as boolean when using them with:

	sdb_bool_set();
	sdb_bool_get();

Complex types:
--------------

See json.md and array.md for more information about the complex types supported in sdb.
