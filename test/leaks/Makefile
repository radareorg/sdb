all: chk
	@./leak.sh ./chk 5
	@./leak.sh ./chk 4
	@./leak.sh ./chk 3
	@./leak.sh ./chk 2
	@./leak.sh ./chk 1
	@./leak.sh ./chk 0

SDBPATH=../../src
CFLAGS+=-I$(SDBPATH)
LIBS+=$(SDBPATH)/libsdb.a

chk: chk.o
	${CC} chk.o -o chk ${LIBS}

.PHONY: chk
