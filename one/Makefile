HASOVR=$(shell clang -Wno-initializer-overrides -E -</dev/null 2>&1| grep unknown)
ifeq ($(HASOVR),)
CFLAGS+=-Wno-initializer-overrides
endif

INCFILES=types.h
INCFILES+=json/rangstr.h
INCFILES+=buffer.h
INCFILES+=ls.h
INCFILES+=ht_uu_cpp.h
INCFILES+=ht_up_cpp.h
INCFILES+=ht_pp_cpp.h
# INCFILES+=sdbht.h
INCFILES+=dict.h
INCFILES+=cdb.h
INCFILES+=cdb_make.h
INCFILES+=json/json.h
INCFILES+=version.h
INCFILES+=sdb.h
IFILES=$(addprefix ../src/,$(INCFILES))

CPPFILES=ht_uu_cpp.h
CPPFILES+=ht_up_cpp.h
CPPFILES+=ht_pp_cpp.h

cpp:
	cpp ../src/ht_uu.h > ../src/ht_uu_cpp.h
	cpp ../src/ht_up.h > ../src/ht_up_cpp.h
	cpp ../src/ht_pp.h > ../src/ht_pp_cpp.h

SRCFILES+=array.c
SRCFILES+=base64.c
SRCFILES+=buffer.c
SRCFILES+=cdb.c
SRCFILES+=cdb_make.c
SRCFILES+=disk.c
SRCFILES+=fmt.c
SRCFILES+=json.c
SRCFILES+=util.c
SRCFILES+=lock.c
SRCFILES+=ls.c
SRCFILES+=ns.c
SRCFILES+=num.c
SRCFILES+=query.c
SRCFILES+=ht_uu.c
SRCFILES+=ht_up.c
SRCFILES+=ht_pp.c
SRCFILES+=json/api.c
SRCFILES+=json/rangstr.c
SRCFILES+=json/indent.c
SRCFILES+=json/js0n.c
SRCFILES+=json/path.c
SRCFILES+=sdb.c
SFILES=$(addprefix ../src/,$(SRCFILES))

all: cpp
	cat $(IFILES) | grep -v 'include "' > sdb-one.h
	(echo '#include "sdb-one.h"'; cat  $(SFILES) | grep -v 'include "') > sdb-one.c
	$(CC) $(CFLAGS) a.c -I.

clean:
	rm -f a.out sdb-one.c sdb-one.h
