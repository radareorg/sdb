/* node sdb test */

var sdb = require ("../nodejs/cc/sdb");
var util = require ("util");

var db = new sdb.open ("db.sdb");
	db.set ("g", 
"{\"glossary\":{\"title\":\"example glossary\",\"page\":1,\"GlossDiv\":{\"title\":\"First gloss title\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\",\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\",\"GlossDef\":{\"para\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\":[\"OK\",\"GML\",\"XML\"]},\"GlossSee\":\"markup\"}}}}} "
);

var i=0;
for(i=0; i<199999; i++) {
	var lala = db.jsonGet ("g", "glossary.title");
}
