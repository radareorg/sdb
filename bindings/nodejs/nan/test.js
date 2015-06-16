/* sdb example */

var sdb = require('bindings')('sdb');

var db = new sdb.SdbObject();
// FAILS 
//var db = sdb.SdbObject();
db.set ("hello", "world");
console.log("Hello "+db.get("hello"));

console.log(sdb.version);

