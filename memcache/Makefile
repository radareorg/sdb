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

${OBJS} mainclient.o: cmds.h

${BIN}: ${OBJS}
ifneq ($(SILENT),)
	@echo BIN ${BIN}
endif
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS} ${LIBS}

cmds.h: cmds.o
	${HOST_CC} -o cmds ../src/util.c cmds.c ${CPPFLAGS} -I../src ${HOST_CFLAGS} ${LDFLAGS}
	./cmds > cmds.h

clean:
	rm -rf *.dSYM
	rm -f *.sdb ${LIBA} ${COBJS} ${CBIN} mainclient.o
	rm -f ${OBJS} ${BIN}cmds.h cmds cmds.o

test:
	./mcsdbd &
	./mcsdbc < test
	killall mcsdbd

.PHONY: clean test all loc
