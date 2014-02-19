#!/usr/bin/newlisp
(load "sdb.lsp")
(setq db (sdb:new 0 "test.sdb" 0))
(sdb:get db "404")
(println "Your old name is '" 
	(sdb:get db "user.name") "'")

; test sdb:query
; (sdb:set db "num" "1")
; (sdb:query db "+num")
; FAIL (print (append "pus" (sdb:get db "num")))

; (print (format "Number %d" (sdb:get db "num")))
(print "Your new name is: ")
(sdb:set db "user.name" (read-line))
; (sdb:set db "user.name" "pancake")
(println "--> " (sdb:get db "user.name") " <--")
(println (format "HASH: %x" (sdb:hash (sdb:get db "user.name"))))

(sdb:sync db)
(sdb:free db)
(exit)
