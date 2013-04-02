#!/usr/bin/python

from SimpleDB import *

db = Sdb("test.sdb", False)
db.set("foo","World",0)
print("Hello "+db.get("foo", None))
db.query("foo=Patata")
print("--> "+db.querys("foo", 0, None))
db.sync()
