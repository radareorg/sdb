# TODO: use parvala
VFILES+=hash.vala 
VFILES+=stack.vala 
VFILES+=array.vala arrayiterator.vala
VFILES+=list.vala listiterator.vala 
VFILES+=iterator.vala sdbinstance.vala serializable.vala
VFILES+=main.vala
OFILES=$(subst vala,o,${VFILES})
CFILES=$(subst vala,c,${VFILES})
VFLAGS=--pkg sdb --vapidir=..
VFLAGS+=-X -I../../src
CFLAGS+=-I../../src
LDFLAGS+=../../src/libsdb.a
CFLAGS+=$(shell pkg-config --cflags gobject-2.0)
LDFLAGS+=$(shell pkg-config --libs gobject-2.0)

all: test
	rm -f food.sdb
	./test
	sdb food.sdb
.PHONY: test

test:
	valac --save-temps -c ${VFILES} ${VFLAGS}
	${CC} -o test ${OFILES} ${LDFLAGS} 

atest: ${OFILES}
	${CC} -o test ${OFILES} ${LDFLAGS}

${CFILES}: ${VFILES}
	valac ${VFLAGS} -C ${VFILES}

${OFILES}: ${CFILES}

clean:
	rm -f *.c *.o test food.sdb
