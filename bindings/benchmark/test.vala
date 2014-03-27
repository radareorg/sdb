using SDB;

void main() {
	Sdb s = new Sdb ();
	s.set ("g", """
{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}} 
""");
	var a = s.json_get ("g", "glossary.title");	
	print ("%s\n", a);

	for (int i = 0; i< 199999; i++) {
		var x = s.json_get ("g", "glossary.title");	
		x = null;
	}
}
