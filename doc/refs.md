References
==========

The Sdb databases uses a reference counting system to identify which Sdb instances must be freed from the namespaces (SdbNs). When you create a new instance it comes with the reference counter variable set to 1. And everytime you add a new reference to it from another place it will increase that counter. In the most common case that would not be used, see the following example:

	Sdb* s = sdb_new (NULL, NULL, 0);
	sdb_free (s);

The first line increments the internal reference counting by 1 and the second substracts one, if it's 0 then proceeds with the freeing stuff.

That's not really useful in this situation but we may probably fall into issues when associating independent instances in a different namespace with `sdb_ns`.

The second scenario will look like this:

	Sdb* s = sdb_new0 ();   // s->refs = 1
	Sdb* r = sdb_new0 ();   // r->refs = 1
	sdb_ns_set (s, "r", r); // r->refs = 2
	sdb_free (s);           // s->refs = 0 r->refs = 1
	sdb_free (r);           // s->refs = 0

Most common sdb namespace usage (without referencing other databases doesnt requires refcounting:

	Sdb* s = sdb_new0 ();
	Sdb* r = sdb_ns (s, "r");
	sdb_free (s);

In the case we are using a copy of the pointer we will need to use `sdb_drain` to transplant the information to make persistent pointers for new databases. This method allows backreferences to keep working as expected even if used by namespaces referencing other databases.

This was a complex explanation, so i'll try to show you an usage example that demonstrates where `sdb_drain` is useful.

	Sdb* s = sdb_new0 ();
	Sdb* r = s;
	sdb_drain (s, sdb_new0 ());
	sdb_const_get (r, "key", 0);

What it's happening here is the following:

1. Create a new sdb instance
2. Create a reference to this one
3. We drain a new database into 's' storage
4. We can still use the old pointer with the new database

The `sdb_drain` call is updating the internals of the database by deinitializing the old one and copying the contents of another one inside the original allocation.

When this call is performed the reference counting is kept from the old instance, so we keep track of the old references. In the case we are using a weak reference like `Sdb* r = s` we should not free that reference unless we do an sdb_ref() on it.
