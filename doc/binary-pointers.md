Saving Pointers
===============

Sometimes you just want to store in Sdb a reference to a native data structure which is allocated by the application. This is useful for using Sdb at runtime.

As long as pointers don't live forever, those keys should have an expiration time, this way, we ensure they are not serialized to disk if we use the same Sdb instance for runtime and offline database.

Pointers should be stringified with the %p format string. And they can be used as in two different ways:

* in keys: for pointer validation and metadata
* in values: for native data reference

In keys
-------

The first use case, consists in storing the pointer in the key like this:

	ptr.0x8049420=Person

This form allows us to know if a pointer is valid (allocated) and retrieve the kind of data is contains. In the above example. We can wrap this in C like this:

	char key[32];
	Person *p = new_Person ();
	snprintf (key, sizeof (key)-1,
		"ptr.%p", p);
	sdb_set (DB, key, "Person", 0);

Then we would like to get that pointer and verify if the type matches:

	const char *type;
	type = sdb_const_get (DB, key, 0);
	if (type && !strcmp (type, "Person")) {
		return 1;
	}
	eprintf ("Invalid pointer\n");
	return 0;


I use to abuse CPP to get fancier syntax for using Sdb from C. I recommend the use of those sugar magic to make the code more readable and easier to maintain without hiding black magic trick.

In addition we can also append a pointer into the value, to store a pointer to alternative data, which can be used for linked lists. 

	ptr.0x8049420=Person,0x8049320

In values
---------
The second method allows us to retrieve a pointer from a key

	fcn.main=0x8048400

We can get the pointer in C like this:

	void *p = (void*)(size_t) \
		sdb_num_get (DB,
		"fcn.main", NULL);

in order to extend the features of , binary data, strings with esotheric encodings.
