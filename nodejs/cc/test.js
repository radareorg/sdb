/* node sdb test */

var sdb = require ("./sdb");
var util = require ("util");
var p = console.log;

console.log (sdb);
var db = new sdb.open ("db.sdb");
//console.log (db);
p ("version: "+sdb.version);
p ("file: "+db.file);
db.set ("g", 
"{\"glossary\":{\"title\":\"example glossary\",\"page\":1,\"GlossDiv\":{\"title\":\"First gloss title\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\",\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\",\"GlossDef\":{\"para\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\":[\"OK\",\"GML\",\"XML\"]},\"GlossSee\":\"markup\"}}}}} "
);

// XXX: quotes are not properly supported
var lala = db.jsonSet ("g", "glossary.title", "this is\" my message");
var lala = db.jsonGet ("g", "glossary.title");
p (lala);
var r = db.set ("foo", "bar");
p (db.get ("foo"));
p (db.get ("cow"));

p (db.exists ("foo"));
p (db.exists ("cow"));

db.sync ();

// var s = new Sdb (null);
