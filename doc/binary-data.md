Base64
======

The standarized way to store binary data in Sdb keys-pairs is by using base64 encoding/decoding. Sdb already provides an API for doing this.

There are several situations where we may want to store binary data inside Sdb. Having in mind about the size restrictions of the key and value. We should not use Sdb to store big chunks of binary data (>10MB), because there are more optimal ways to do this (filesystem?) but it may be useful for caching images, array contents, data structures, etc.

The Sdb.Encode and Sdb.Decode static functions are the ones responsible for doing the base64 conversions. Here's a simple usage example of those functions.

	#define DB core->sdb
	int i, abc_len = 'z'-'a'+1;
	char *str, abc[abc_len];
	for (i=0; i<abc_len;i++)
	    abc[i] = 'a'+i;
	abc[i] = 0;
	str = sdb_encode (abc, abc_len);
	sdb_set (DB, "abc", str, 0);
	free (str);

	str = sdb_decode (
	    sdb_const_get (
	        DB, "abc", 0), -1);
	printf ("> %s\n", str);

Note that if we pass -1 to Sdb.Decode it will perform an strlen() to the first argument to determine its length. If the value is >=0, then it asumes that size to be read from the pointer passed as first argument.


The '%' operator in Sdb.Query can be used to encode or decode an Sdb key from the shell. This eases the usage from commandline for data extraction.

	> %foo=bar

	> foo
	YmFy

	> %foo
	bar
