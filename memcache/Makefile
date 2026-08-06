include ../config.mk

BIN=mcsdbd
CBIN=mcsdbc
NETTEST=test_net
OBJS=main.o memcache.o net.o protocol.o client.o
COBJS=client.o net.o memcache.o
LIBS=../src/libsdb.a
LIBA=libmcsdb.a
CFLAGS+=-I../include
CFLAGS+= -g -ggdb

all: ${BIN} ${CBIN} libmcsdb.a

${LIBA}: ${COBJS}
ifneq ($(SILENT),)
	@echo AR ${LIBA}
endif
	${AR} -r ${LIBA} ${COBJS}
	${RANLIB} ${LIBA}

%.o: %.c
ifneq ($(SILENT),)
	@echo CC $<
endif
	${CC} -c ${CPPFLAGS} ${CFLAGS} ${CFLAGS_SHARED} -o $@ $<

${CBIN}: ${COBJS} mainclient.o
ifneq ($(SILENT),)
	@echo BIN ${CBIN}
endif
	${CC} mainclient.o ${COBJS} ${LDFLAGS} -o ${CBIN} ${LIBS}

${OBJS} mainclient.o test_net.o: cmds.h

${BIN}: ${OBJS}
ifneq ($(SILENT),)
	@echo BIN ${BIN}
endif
	${CC} -o ${BIN} $(CFLAGS) ${OBJS} ${LDFLAGS} ${LIBS}

cmds.h: cmds.c
	${HOST_CC} -o cmds cmds.c ${CPPFLAGS} $(CFLAGS) ${HOST_CFLAGS} ${LDFLAGS}
	./cmds > cmds.h

${NETTEST}: test_net.o net.o
	${CC} test_net.o net.o ${LDFLAGS} -o ${NETTEST}

check: ${NETTEST}
	./${NETTEST}

clean:
	rm -rf *.dSYM
	rm -f *.sdb ${LIBA} ${COBJS} ${CBIN} mainclient.o ${NETTEST} test_net.o
	rm -f ${OBJS} ${BIN} cmds.h cmds cmds.o

test:
	./mcsdbd &
	./mcsdbc < test
	killall mcsdbd

.PHONY: clean test check all loc
