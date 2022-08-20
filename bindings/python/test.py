#!/usr/bin/env python3

import r2sdb

s = r2sdb.new()
s.set("foo", "bar")
a = s.get("foo")
print(a)
a = s.get("bar")
print(a)
s.set("world", "loops")
print(s.query("*"))
