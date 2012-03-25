var item = '{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}}';

console.log (item);
var container = {};

container.item = item;

for (var i=0; i<199999; i++) {
	var a = container.item;
	var obj = JSON.parse (item);
	var title = obj.glossary.title;
}
