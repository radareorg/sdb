#!/usr/bin/env node
var Sdb = require ("./");

var s = Sdb.open ("test.sdb");
s.add ("foo", "Hello World");
console.log (s.get ("foo"));
s.sync ();

var js = s.json_indent ('{"hello":"world","foo":{"foo":1,"bar":2}}');

js = '{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}}';

js = s.json_indent (js);
console.log (js);

s.set ("g", js);

var a = s.json_get ("g", 'glossary.GlossDiv.GlossList.GlossEntry.GlossDef.GlossSeeAlso[0]');
console.log ("==> ",a);
// 3 times slower than luajit-ffi!
if (false) {
	for (var i=0; i<199999; i++) {
		var a = s.json_get ("g", "glossary.title");
	}
}

// works
s.query ("a=win;a")

// works
console.log ("___ "+s.querys ("a=win;a"));
