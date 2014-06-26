Binary Structs
==============

Applications use to need some kind of data persistence between runs, Sdb supports data serialization and allows the application to save binary data structures to disk for later recovery.

It is important to say that those structures can't contain pointers. This restriction is clear for the reason that pointers are not going to be valid between different executions of the program, so the structs should contain only basic types and arrays.

Using the Sdb.Fmt API we can achieve that struct <-> string serializations. It handles a format-string that its applied on a memory pointer to generate a string that represents the contents of the struct.

The following code serves as an example for using this API to serialize a native struct into a string and bring it back to memory.

	typedef struct person {
		int foo;
		char *str;
		ut64 fin;
		int end;
	} Person;

	Person p;

	sdb_fmt_init (&p, "dsqd");
	sdb_fmt_tobin ("123,bar,321,1", "dsqd", &p);
	eprintf ("--> %d,%s\n", p.foo, p.str);
	eprintf ("--> %lld,%d\n", p.fin, p.end);

	{
		char *o = sdb_fmt_tostr (&p, "dsqd");
		eprintf ("== %s\n", o);
		free (o);
	}
	sdb_fmt_free (&p, "dsqd");
