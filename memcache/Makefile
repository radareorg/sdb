include ../config.mk

BIN=mcsdbd
CBIN=mcsdbc
OBJS=main.o memcache.o net.o protocol.o client.o
COBJS=client.o net.o memcache.o
LIBS=../src/libsdb.a
LIBA=libmcsdb.a
CFLAGS+=-I../src

all: ${BIN} ${CBIN} libmcsdb.a

${LIBA}: ${COBJS}
	ar -r ${LIBA} ${COBJS}
	ranlib ${LIBA}

${CBIN}: ${COBJS} mainclient.o
	${CC} mainclient.o ${COBJS} ${LIBS} -o ${CBIN}

${OBJS}: cmds.h

${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS} ${LIBS}

cmds.h: cmds.o
	${CC} -o cmds cmds.o ${CFLAGS} ${LDFLAGS} ${LIBS}
	./cmds > cmds.h

clean:
	rm -f *.sdb
	rm -f ${LIBA}
	rm -f ${COBJS} ${CBIN} mainclient.o
	rm -f ${OBJS} ${BIN}
	rm -f cmds.h cmds cmds.o

test:
	./mcsdbd &
	./mcsdbc < test
	killall mcsdbd

.PHONY: clean test all loc
