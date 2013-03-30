include ../config.mk

BIN=mcsdbd
CBIN=mcsdbc
OBJS=main.o memcache.o net.o protocol.o client.o
COBJS=client.o net.o memcache.o
LIBS=../src/libsdb.a
LIBA=libmcsdb.a
CFLAGS+=-I../src
CFLAGS+= -g -ggdb

all: ${BIN} ${CBIN} libmcsdb.a

${LIBA}: ${COBJS}
	${AR} -r ${LIBA} ${COBJS}
	${RANLIB} ${LIBA}

${CBIN}: ${COBJS} mainclient.o
	${CC} ${LIBS} mainclient.o ${COBJS} ${LDFLAGS} -o ${CBIN}

${OBJS} mainclient.o: cmds.h

${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS} ${LIBS}

cmds.h: cmds.o
	${HOST_CC} -o cmds ../src/util.c cmds.c ${CPPFLAGS} ${CFLAGS} ${LDFLAGS}
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
