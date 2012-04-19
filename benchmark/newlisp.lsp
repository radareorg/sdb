#!/usr/bin/newlisp
; hello world in newlisp using sdb

(load "../newlisp/sdb.lsp")
(setq json " {\"glossary\":{\"title\":\"example glossary\",\"page\":1,\"GlossDiv\":{\"title\":\"First gloss title\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\",\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\",\"GlossDef\":{\"para\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\":[\"OK\",\"GML\",\"XML\"]},\"GlossSee\":\"markup\"}}}}}'")
; (print json)
; (print (sdb:jsonIndent json) "\n")

(setq db (sdb:new "test.sdb" 0))

(sdb:set db "g" json) ; "{\"hello\":\"world\"}")
; (sdb:set db "g" "{\"hello\":\"world\"}")
; (dotimes (i 199999)
(for (i 0 199999)
	(setq v (sdb:jsonGet db "g" "glossary.title"))
;	(print i " " v " <--\n")
)
(sdb:sync db)
(sdb:free db)
(exit 0)
