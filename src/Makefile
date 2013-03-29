include ../config.mk

CFLAGS+= -g -ggdb
#CFLAGS+=-DOLDFMT=1
OBJ=cdb.o buffer.o cdb_make.o ls.o ht.o sdb.o sdbn.o sdba.o query.o
OBJ+=json.o json/js0n.o json/json.o json/rangstr.o ns.o lock.o

SOBJ=$(subst .o,.so,${OBJ})

BIN=sdb${EXEXT}

.PHONY: all static shared clean

all: ${BIN}
	${MAKE} static
	${MAKE} shared

sdb-version.h:
	cd .. ; ${MAKE} src/sdb-version.h

shared: sdb-version.h
	${MAKE} libsdb.${SOEXT}

static: sdb-version.h
	${MAKE} libsdb.a

libsdb.a: ${OBJ}
	rm -f libsdb.a
	${AR} -r libsdb.a ${OBJ}
	${RANLIB} libsdb.a

%.so: %.c
	${CC} -c ${CPPFLAGS} ${CFLAGS} ${CFLAGS_SHARED} -o $@ $^

libsdb.${SOEXT}: ${SOBJ}
	echo JOJO
	${CC} ${LDFLAGS} $(LDFLAGS_SHARED) -o $@ ${SOBJ}

${BIN}: libsdb.a main.o
	${CC} ${LDFLAGS} -o ${BIN} main.o ${OBJ}

clean:
	rm -rf ${OBJ} ${SOBJ} main.o libsdb.a a.out sdb sdb.dSYM *.sdb *.db *.${SOEXT}
