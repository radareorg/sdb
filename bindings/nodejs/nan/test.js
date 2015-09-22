/* sdb example */

var sdb = require('bindings')('sdb');
/*
// TODO: beautify the constructor
var db = sdb ("test.db");
var db = sdb.open("test.db");
*/
var db = new sdb.Database("test.db");

console.log ("Keys: "+db.keys_on_disk());

if (sdb.json) {
	const J = sdb.json;
	console.log(J.indent('{"pop":33,"caca":123}'));
}

if (sdb.json_indent) {
	console.log (
		sdb.json_unindent (
			sdb.json_indent (
				'{"pop":33,"caca":123}'
			)
		)
	);
}

db.set("fooormiga", "1")
db.set("fooonambug", "2")
db.set("fulormiga", "3")
db.set("furuBUGmi", "4")

console.log("------------");
var res = db.like ("^foo");
//var res = db.like (/bug/i);
console.log(JSON.stringify (res));

console.log("------------");

console.log(sdb.version);
// FAILS 
//var db = sdb.SdbObject();

db.set ("hello", "world");
db.set("hello", 33);
console.log("Hello "+db.get("hello"));

try {
  var pop = db.get("pop")
  db.set("pop","bar")
    .set("syn","fin")
    .add("oyn","jol") // will fail here
    .set("xin","min");
  console.log ("ERROR (expecting fail)", e);
} catch (e) {
  console.error ("OK (expected fail)", e);
}

console.log(db.sync());
console.log(db.query("a=c;a"))
