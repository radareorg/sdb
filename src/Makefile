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
ifneq ($(SILENT),)
	@echo AR libsdb.a
endif
	rm -f libsdb.a
	${AR} -r libsdb.a ${OBJ}
	${RANLIB} libsdb.a

libsdb.${SOEXT}: ${SOBJ}
ifneq ($(SILENT),)
	@echo LD libsdb.${SOEXT}
endif
	${CC} ${LDFLAGS} $(LDFLAGS_SHARED) -o $@ ${SOBJ}

${BIN}: libsdb.a main.o
	${CC} ${LDFLAGS} -o ${BIN} main.o ${OBJ}

clean:
	rm -rf ${OBJ} ${SOBJ} main.o libsdb.a a.out sdb sdb.dSYM *.sdb *.db *.${SOEXT}

# rules #
.c:
ifneq ($(SILENT),)
	@echo LD $<
endif
	$(CC) $(LDFLAGS) -c $(CFLAGS) -o $@ $<

.c.o:
ifneq ($(SILENT),)
	@echo CC $(shell basename $<)
endif
	$(CC) -c $(CFLAGS) -o $@ $<

%.so: %.c
ifneq ($(SILENT),)
	@echo SO $<
endif
	${CC} -c ${CPPFLAGS} ${CFLAGS} ${CFLAGS_SHARED} -o $@ $^
