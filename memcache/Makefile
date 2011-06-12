BIN=mcsdbd
OBJS=main.o memcache.o net.o protocol.o
LIBS=../src/libsdb.a
CFLAGS+=-I../src

CFLAGS+=-ggdb -g -O0 -Wall
LDFLAGS+=-ggdb -g

all: ${BIN} client

client: client.o net.o memcache.o
	${CC} client.o memcache.o net.o ${LIBS} -o client

${OBJS}: cmds.h

${BIN}: ${OBJS}
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
