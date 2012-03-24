var Sdb = require ("./sdb");

var s = Sdb.open ("test.sdb");
s.add ("foo", "Hello World");
console.log (s.get ("foo"));
s.sync ();
