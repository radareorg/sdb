; TODO: add support for CAS

(context 'sdb)

(constant 'SDBLIB
	(or (env "SDBLIB")
		(if (= ostype "OSX")
			"/usr/lib/libsdb.dylib" ; osx
			"/usr/lib/libsdb.so"))) ; linux

(import SDBLIB "sdb_new")
(import SDBLIB "sdb_set")
(import SDBLIB "sdb_add")
(import SDBLIB "sdb_num_inc")
(import SDBLIB "sdb_num_dec")
(import SDBLIB "sdb_get")
(import SDBLIB "sdb_sync")
(import SDBLIB "sdb_free")
(import SDBLIB "sdb_query")
(import SDBLIB "sdb_querys")
(import SDBLIB "sdb_array_length")
(import SDBLIB "sdb_array_delete")
(import SDBLIB "sdb_array_get")
(import SDBLIB "sdb_array_set")
(import SDBLIB "sdb_array_insert")
(import SDBLIB "sdb_json_get")
(import SDBLIB "sdb_json_set")
(import SDBLIB "sdb_json_num_inc")
(import SDBLIB "sdb_json_num_dec")
(import SDBLIB "sdb_json_indent")
(import SDBLIB "sdb_json_unindent")

(define (type x)
  (let (types
         '("bool" "bool" "integer" "float"
           "string" "symbol" "context" "primitive"
           "cdecl" "stdcall" "quote" "list" "lambda"
           "macro" "array"))
    (types (& 0xf ((dump x) 1)))))

; query
(define (sdb:query db var)
	(sdb_query db var))
(define (sdb:querys db var)
	(define ret (sdb_querys db nil 0 var))
	(if (= ret 0) nil (get-string ret)))
; arrays
(define (sdb:alength db key)
	(sdb_array_length db key))
(define (sdb:aset db key idx val)
	(sdb_array_set db key idx val 0))
(define (sdb:ains db key idx val)
	(sdb_array_insert db key idx val 0))
(define (sdb:adel db key idx)
	(sdb_array_delete db key idx 0))
(define (sdb:aget db key idx)
	(define ret (sdb_array_get db key idx nil))
	(if (= ret 0) nil (get-string ret)))
; sdb
(define (sdb:hash s)
	; sdb_hash is not exported by the shared library, reimplement the default DJB2-style algorithm
	(let ((h 5381))
		(dostring (c s)
			(setq h (& 0xffffffff (^ (+ (<< h 5) h) c))))
		h))
(define (sdb:new path file lock)
	(sdb_new path file lock))
(define (sdb:set db var val)
	(sdb_set db var val 0))
(define (sdb:add db var val)
	(sdb_add db var val 0))
(define (sdb:inc db var val)
	(sdb_num_inc db var val 0))
(define (sdb:dec db var val)
	(sdb_num_dec db var val 0))
(define (sdb:get db var)
	(define ret (sdb_get db var 0))
	(if (= ret 0) nil (get-string ret)))
(define (sdb:free db)
	(sdb_free db))
(define (sdb:sync db)
	(sdb_sync db))
; json
(define (sdb:jsonGet db key path)
	(let (ret (sdb_json_get db key path 0))
		(if (= 0 ret) "" (get-string ret))))
; TODO: fix return value, optional cas
(define (sdb:jsonInc db key path value cas)
	(sdb_json_num_inc db key path value cas))
; TODO: fix return value, optional cas
(define (sdb:jsonDec db key path value cas)
	(sdb_json_num_dec db key path value cas))
(define (sdb:jsonSet db key path value)
	(sdb_json_set db key path value 0))
(define (sdb:jsonIndent js)
	(let (ret (sdb_json_indent js "  "))
		(if (= 0 ret) "" (get-string ret))))
(define (sdb:jsonUnindent js)
	(let (ret (sdb_json_unindent js))
		(if (= 0 ret) "" (get-string ret))))

(context 'MAIN)
