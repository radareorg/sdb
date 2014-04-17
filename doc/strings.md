Strings
=======

Everything inside sdb is a string, those are null terminated sequence of one byte characters.

base64
------
Strings in sdb can be encoded in base64 and the `sdb_encode()` and `sdb_decode()`. This encoding will allow you to store text without any limitation of encoding, newlines, tabs, commas or any other special character.

The `sdb_query()` interface supports the '%' prefix character to perform an `sdb_encode()` on set and `sdb_decode()` on get.
