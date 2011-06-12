include ../config.mk

BIN=mcsdbd
OBJS=main.o memcache.o net.o protocol.o
LIBS=../src/libsdb.a
CFLAGS+=-I../src

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

test:
	./mcsdbd &
	./client < test
	killall mcsdbd

.PHONY: clean test all loc
