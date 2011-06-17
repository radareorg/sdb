# TODO: use parvala
VFILES+=hash.vala 
VFILES+=stack.vala 
VFILES+=list.vala listiterator.vala 
VFILES+=iterable.vala iterator.vala sdbinstance.vala 
VFILES+=main.vala
OFILES=$(subst vala,vala.o,${VFILES})
CFILES=$(subst vala,c,${VFILES})
VFLAGS=--pkg sdb --vapidir=..
CFLAGS+=-I../../src
LDFLAGS+=../../src/libsdb.a
CFLAGS+=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS+=$(shell pkg-config --libs gobject-2.0)

all: test
.PHONY: test

test:
	valac -c ${VFILES} ${VFLAGS}
	${CC} -o test ${OFILES} ${LDFLAGS} 

atest: ${OFILES}
	${CC} -o test ${OFILES} ${LDFLAGS}

${CFILES}: ${VFILES}
	valac ${VFLAGS} -C ${VFILES}

${OFILES}: ${CFILES}

clean:
	rm -f *.c *.o test food.sdb
