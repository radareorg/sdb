Namespaces
==========

Namespaces are used to instantiate new Sdb databases in a tree structure linked from a root Sdb.

Sdb namespaces can be traversed using the '/' character in the keys using the `sdb_query` APIs.

	$ sdb - foo/bar/cow=3 foo/bar/*
	cow=3
	$ sdb - foo/bar/cow=3 ***
	foo
	foo/bar

The three asterisks is a magic key used to retrieve a list of all the namespaces recursively (except the root). The commands are the following:

- `*` list all keys in selected ns
- `**` list all namespaces on selected ns
- `***` list recursively all keys in selected ns

Namespaces can be created or defined from an already existing Sdb instance.

	Sdb* s = sdb_new0 ();
	Sdb* S = sdb_new0 ();
	sdb_ns_set (s, "r", S);
	Sdb* R = sdb_ns (s, "r");
	assert (r == S)

Using it from Vala or other language bindings:

	Sdb sdb = new Sdb();
	sdb.ns("country").ns("city").num_set ("barcelona", 184957);


Questions:
----------

??? The '/' magic should maybe happen at sdb_get level, becase '/' is an invalid char for keys.
