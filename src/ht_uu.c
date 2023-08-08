/* sdb - MIT - Copyright 2018-2023 - ret2libc, pancake */

#include "sdb/sdb.h"
#include "sdb/ht_uu.h"
#include "ht.inc.c"

// creates a default HtUU that has strings as keys
SDB_API HtName_(Ht)* Ht_(new0)(void) {
	HT_(Options) opt = {
		.cmp = NULL,
		.hashfn = NULL,
		.dupkey = NULL,
		.dupvalue = NULL,
		.calcsizeK = NULL,
		.calcsizeV = NULL,
		.freefn = NULL
	};
	return Ht_(new_opt) (&opt);
}
