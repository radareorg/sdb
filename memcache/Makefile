BIN=mcsdbd
OBJS=main.o memcache.o net.o protocol.o
LIBS=../src/libsdb.a
CFLAGS+=-I../src

CFLAGS+=-ggdb -O3 -Wall
LDFLAGS+=-ggdb

all: ${BIN}

${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS} ${LIBS}

clean:
	rm -f ${OBJS} ${BIN}

loc:
	wc -l *.c

test:
	time ./mcsdbd < test

.PHONY: clean test all
