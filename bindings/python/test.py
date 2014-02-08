#!/usr/bin/python

from SimpleDB import *

db = Sdb(None, "test.sdb", False)
db.set(b"foo",b"World",0)
print("Hello "+db.get(b"foo", None))
db.query(b"foo=Patata")
print("--> "+db.querys((b"foo").decode("utf-8"), 0, None))
db.sync()
