#!/usr/bin/env node
var Sdb = require ("../nodejs/ffi/sdb");

var s = Sdb.open ();


var js = '{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}}';

s.set ("g", js);

var a = s.jsonGet ("g", 'glossary.GlossDiv.GlossList.GlossEntry.GlossDef.GlossSeeAlso[0]');
console.log (a);
// 3 times slower than luajit-ffi!
if (true) {
	for (var i=0; i<199999; i++) {
		var a = s.jsonGet ("g", "glossary.title");
	}
}
