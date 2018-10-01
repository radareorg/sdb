SRCDIR=../../src
BASEDIR?=${SRCDIR}
SDB="${BASEDIR}/sdb"
CFLAGS+=-I${SRCDIR} -I${BASEDIR}
LDFLAGS+=-static -L${BASEDIR} -lsdb

all:
	rm -f db.test
	${SDB} db.test foo=old
	$(CC) test-reset.c -o test-reset $(CFLAGS) $(LDFLAGS)
	./test-reset

asan:
	$(MAKE) CFLAGS="-fsanitize=address ${CFLAGS}"

clean:
	rm -f a.out test-reset db.test test.db
