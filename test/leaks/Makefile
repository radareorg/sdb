all: chk qry
	@./leak.sh ./qry 3
	@./leak.sh ./qry 2
	@./leak.sh ./qry 1
	@./leak.sh ./qry 0
	#
	@./leak.sh ./chk 6
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

qry: qry.o
	${CC} qry.o -o qry ${LIBS}

.PHONY: chk
