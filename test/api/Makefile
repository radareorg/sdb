SRCDIR=../../src
BASEDIR?=${SRCDIR}
CFLAGS+=-I${SRCDIR} -I${BASEDIR}
LDFLAGS+=-static -L${BASEDIR} -lsdb

all: array refs
	./array
	./refs

array: array.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

refs: refs.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f ./array ./refs
