#!/usr/bin/newlisp
(load "sdb.lsp")
(setq db (sdb:new "test.sdb" 0))
(sdb:get db "404")
(println "Your old name is '" 
	(sdb:get db "user.name") "'")
(print "Your new name is: ")
(sdb:set db "user.name" (read-line))
; (sdb:set db "user.name" "pancake")
(println "--> " (sdb:get db "user.name") " <--")
(sdb:sync db)
(sdb:free db)
(println (format "HASH: %d" (sdb:hash (sdb:get db "user.name"))))
(exit)
