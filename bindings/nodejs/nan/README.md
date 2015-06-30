SDB bindings for nodejs
=======================

SDB is a fast and small key-value database.

All keys and values can only be strings, those strings are
interpreted as arrays, numbers, booleans, pointers, or even
JSON objects that can be parsed and indented pretty quickly.

The SDB database works like memcache, as it is designed to
run in memory, but supports atomic sync to disk and journaling.

Getting Started
===============

```js
var sdb = require('sdb');
var db = new sdb.Database("test.db");
// quick json indentation
console.log(sdb.json_unindent(
  sdb.json_indent('{"pop":33,"caca":123}')));
db.set("foo", "bar")
console.log ("Hello "+db.get("bar"));
```
