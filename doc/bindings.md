Bindings
========

D
-

Go
--

CSharp
------

NewLisp
-------

Vala
----

Python
------

This is a sample program using the Python API for SDB.

    #!/usr/bin/python
    from SimpleDB import *
    db = Sdb(None, "test.sdb", False)
    db.set("foo", "World",0)
    print("Hello "+db.get("foo", None))
    db.query(b"foo=Patata")
    #print("--> "+db.querys(None, 0, ("foo").decode("utf-8"), 0, None))
    print("--> "+db.querys(None, 0, "foo"))
    db.sync()

NodeJS
------

