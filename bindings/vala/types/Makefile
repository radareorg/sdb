PFX?=/usr
PWD=$(shell pwd)

include ../../../config.mk
INSTALL_DATA?=install -m 644

VALAC?=valac
VFILES+=hash.vala
VFILES+=stack.vala stackiterator.vala
VFILES+=array.vala arrayiterator.vala
VFILES+=list.vala listiterator.vala 
VFILES+=iterator.vala serializable.vala iterable.vala
OFILES=$(subst vala,o,${VFILES})
CFILES=$(subst vala,c,${VFILES})
VFLAGS+=-X -I../../../src
VFLAGS+=../sdb.vapi
CFLAGS+=-I../../../src
LDFLAGS+=../../../src/libsdb.a
CFLAGS+=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS+=$(shell pkg-config --libs gobject-2.0)

ifeq (${BUILD_MEMCACHE},1)
VFLAGS+=../mcsdb.vapi
CFLAGS+=-I../../../memcache
VFILES+=mcsdbinstance.vala
VFLAGS+=-X -I../../../memcache
LDFLAGS+=../../../memcache/libmcsdb.a
else
VFILES+=sdbinstance.vala
endif

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
	mkdir -p ${DESTDIR}${INCDIR}
	mkdir -p ${DESTDIR}${LIBDIR}/pkgconfig
	mkdir -p ${DESTDIR}${DATADIR}/vala/vapi

install: install-dirs
	$(INSTALL_DATA) sdbtypes.h ${DESTDIR}${INCDIR}
	$(INSTALL_DATA) libsdbtypes.a ${DESTDIR}${LIBDIR}
	$(INSTALL_DATA) sdbtypes.pc ${DESTDIR}${LIBDIR}/pkgconfig
	$(INSTALL_DATA) sdbtypes.vapi ${DESTDIR}${DATADIR}/vala/vapi

symstall: install-dirs
	ln -fs ${PWD}/sdbtypes.h ${DESTDIR}${INCDIR}/sdbtypes.h
	ln -fs ${PWD}/libsdbtypes.a ${DESTDIR}${LIBDIR}/libsdbtypes.a
	ln -fs ${PWD}/sdbtypes.pc ${DESTDIR}${LIBDIR}/pkgconfig/sdbtypes.pc
	ln -fs ${PWD}/sdbtypes.vapi ${DESTDIR}${DATADIR}/vala/vapi/sdbtypes.vapi

uninstall deinstall:
	rm -f ${DESTDIR}${INCDIR}/sdbtypes.h
	rm -f ${DESTDIR}${LIBDIR}/libsdbtypes.a
	rm -f ${DESTDIR}${LIBDIR}/pkgconfig/sdbtypes.pc
	rm -f ${DESTDIR}${DATADIR}/vala/vapi/sdbtypes.vapi

clean:
	rm -f *.c *.o sdbtypes.vapi sdbtypes.h
	rm -f test food.sdb libsdbtypes.a

.PHONY: all run clean install uninstall deinstall symstall install-dirs testvapi
.INTERMEDIATE: intermediate
