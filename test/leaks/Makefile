all: chk qry jsn
	@echo JSN
	@./leak.sh ./jsn 6
	@./leak.sh ./jsn 5
	@./leak.sh ./jsn 4
	@./leak.sh ./jsn 3
	@./leak.sh ./jsn 2
	@./leak.sh ./jsn 1
	@./leak.sh ./jsn 0
	#
	@echo QRY
	@./leak.sh ./qry 3
	@./leak.sh ./qry 2
	@./leak.sh ./qry 1
	@./leak.sh ./qry 0
	#
	@echo CHK
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

jsn: jsn.o
	${CC} jsn.o -o jsn ${LIBS}

chk: chk.o
	${CC} chk.o -o chk ${LIBS}

qry: qry.o
	${CC} qry.o -o qry ${LIBS}

.PHONY: chk jsn qry
