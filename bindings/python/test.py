#!/usr/bin/python

from sdb import *

db = Sdb(None, "test.sdb", False)
db.set("foo", "World",0)
print("Hello "+db.get("foo", None))
db.query(b"foo=Patata")
#print("--> "+db.querys(None, 0, ("foo").decode("utf-8"), 0, None))
#print("--> "+db.querys(None, 0, "foo"))
db.sync()
