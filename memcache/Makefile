BIN=mcsdbd
OBJS=main.o memcache.o net.o protocol.o client.o
LIBS=../src/libsdb.a
CFLAGS+=-I../src

CFLAGS+=-ggdb -O3 -Wall
LDFLAGS+=-ggdb

all: ${BIN}

${BIN}: cmds.h ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS} ${LIBS}

cmds.h: cmds.o
	${CC} -o cmds cmds.o ${CFLAGS} ${LDFLAGS} ${LIBS}
	./cmds > cmds.h

clean:
	rm -f ${OBJS} ${BIN} cmds.h cmds cmds.o

loc:
	wc -l *.c

test:
	time ./mcsdbd < test

.PHONY: clean test all loc
