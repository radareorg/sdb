PFX?=/usr

VFILES+=hash.vala 
VFILES+=stack.vala stackiterator.vala
VFILES+=array.vala arrayiterator.vala
VFILES+=list.vala listiterator.vala 
VFILES+=iterator.vala sdbinstance.vala serializable.vala iterable.vala
OFILES=$(subst vala,o,${VFILES})
CFILES=$(subst vala,c,${VFILES})
VFLAGS=--pkg sdb --vapidir=..
VFLAGS+=-X -I../../src
CFLAGS+=-I../../src
LDFLAGS+=../../src/libsdb.a
CFLAGS+=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS+=$(shell pkg-config --libs gobject-2.0)

all: test libsdbtypes.a

run:
	rm -f food.sdb
	./test
	sdb food.sdb

libsdbtypes.a: ${VFILES}
	ar -r libsdbtypes.a ${OFILES}
	ranlib libsdbtypes.a

testvapi:
	valac --pkg sdbtypes main.vala

test: ${VFILES} main.vala
	valac -H sdbtypes.h --vapi=sdbtypes.vapi --save-temps -c ${VFILES} ${VFLAGS} main.vala
	${CC} -o test ${OFILES} ${LDFLAGS} main.o

install:
	mkdir -p ${PFX}/include
	cp -f sdbtypes.h ${PFX}/include
	mkdir -p ${PFX}/lib/pkgconfig
	cp -f libsdbtypes.a ${PFX}/lib
	cp -f sdbtypes.pc ${PFX}/lib/pkgconfig
	mkdir -p ${PFX}/share/vala/vapi
	cp -f sdbtypes.vapi ${PFX}/share/vala/vapi

clean:
	rm -f *.c *.o test food.sdb libsdbtypes.a

.PHONY: all run clean install
