
#include "sdb.h"
int main () {
	int i;
#define k "g"
#define p "glossary.title"
	Sdb *sdb = sdb_new (NULL, NULL, 0);
sdb_set (sdb, "g",

"{\"glossary\":{\"title\":\"example glossary\",\"page\":1,\"GlossDiv\":{\"title\":\"First gloss title\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\",\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\",\"GlossDef\":{\"para\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\":[\"OK\",\"GML\",\"XML\"]},\"GlossSee\":\"markup\"}}}}}"
, 0);
	for (i=0; i< 199999; i++) {
		free (sdb_json_get (sdb, k, p, NULL));
	}
	sdb_free (sdb);
	return 0;
}
