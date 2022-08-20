#!/usr/bin/env python3

import sdb

s = sdb.new()
s.set("foo", "bar")
a = s.get("foo")
print(a)
a = s.get("bar")
print(a)

#db = sdb.open("test.sdb")
#print(db)

#db.set("hello", "world")
#s = db.get("hello")
#print(s)
#db.close()

#print(sdb.hello());
# help(sdb);
