from SimpleDB import *

db = Sdb("test.sdb", False)
db.set("foo","World",0)
print ("Hello "+db.get("foo", None))
db.sync()
