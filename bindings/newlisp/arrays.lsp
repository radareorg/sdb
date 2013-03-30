#!/usr/bin/newlisp

(load "sdb.lsp")
(setq db (sdb:new "test.sdb" 0))

(sdb:query db "()list=1,2,3,4,5")
(setq str (sdb:querys db "list"))

(print "LEN> " (sdb:alength db "list") "\n")
(print "---> " str "\n")
(print "-0-> " (sdb:aget db "list" 0) "\n")
(sdb:aset db "list" 0 "FUCK")
(print "-0-> " (sdb:aget db "list" 0) "\n")
(sdb:sync db)
(sdb:free db)
(exit)
