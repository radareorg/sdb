PFX?=/usr
PWD=$(shell pwd)

include ../../../config.mk
INSTALL_DATA?=install -m 644

VALAC?=valac
VFILES+=hash.vala 
VFILES+=stack.vala stackiterator.vala
VFILES+=array.vala arrayiterator.vala
VFILES+=list.vala listiterator.vala 
VFILES+=iterator.vala sdbinstance.vala serializable.vala iterable.vala
OFILES=$(subst vala,o,${VFILES})
CFILES=$(subst vala,c,${VFILES})
VFLAGS+=-X -I../../../src
VFLAGS+=-X -I../../../memcache
VFLAGS+=../sdb.vapi ../mcsdb.vapi
CFLAGS+=-I../../../src
CFLAGS+=-I../../../memcache
LDFLAGS+=../../../src/libsdb.a
LDFLAGS+=../../../memcache/libmcsdb.a
CFLAGS+=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS+=$(shell pkg-config --libs gobject-2.0)

all: test

run:
	rm -f food.sdb
	./test
	sdb food.sdb

libsdbtypes.a: ${OFILES}
	$(AR) -r libsdbtypes.a ${OFILES}
	$(RANLIB) libsdbtypes.a

testvapi:
	valac --pkg sdbtypes main.vala

${OFILES}: intermediate

intermediate: ${VFILES} main.vala
	${VALAC} -H sdbtypes.h --vapi=sdbtypes.vapi --save-temps -c ${VFILES} ${VFLAGS} main.vala

test: libsdbtypes.a main.vala
	${CC} -o test ${OFILES} ${LDFLAGS} main.o

install-dirs:
	mkdir -p ${PFX}/include
	mkdir -p ${PFX}/lib/pkgconfig
	mkdir -p ${PFX}/share/vala/vapi

install: install-dirs
	$(INSTALL_DATA) sdbtypes.h ${PFX}/include
	$(INSTALL_DATA) libsdbtypes.a ${PFX}/lib
	$(INSTALL_DATA) sdbtypes.pc ${PFX}/lib/pkgconfig
	$(INSTALL_DATA) sdbtypes.vapi ${PFX}/share/vala/vapi

symstall: install-dirs
	ln -fs ${PWD}/sdbtypes.h ${PFX}/include/sdbtypes.h
	ln -fs ${PWD}/libsdbtypes.a ${PFX}/lib/libsdbtypes.a
	ln -fs ${PWD}/sdbtypes.pc ${PFX}/lib/pkgconfig/sdbtypes.pc
	ln -fs ${PWD}/sdbtypes.vapi ${PFX}/share/vala/vapi/sdbtypes.vapi

uninstall deinstall:
	rm -f ${PFX}/include/sdbtypes.h
	rm -f ${PFX}/lib/libsdbtypes.a
	rm -f ${PFX}/lib/pkgconfig/sdbtypes.pc
	rm -f ${PFX}/share/vala/vapi/sdbtypes.vapi

clean:
	rm -f *.c *.o sdbtypes.vapi sdbtypes.h
	rm -f test food.sdb libsdbtypes.a

.PHONY: all run clean install uninstall deinstall symstall install-dirs testvapi
.INTERMEDIATE: intermediate
