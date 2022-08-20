#!/usr/bin/env python3

import sdb

db = sdb.open("test.sdb")
print(db)

#db.set("hello", "world")
#s = db.get("hello")
#print(s)
#db.close()

print(sdb.hello());
# help(sdb);
