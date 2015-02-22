.PHONY: all reset

BINS=fmt stack cas hook hook2 verypush stress1 fmtarr drain ns nsabuse
BINS+=bench-expire expire dumptwice
CFLAGS=-I../src
LDFLAGS=../src/libsdb.a

all: $(BINS) reset
	@suite/run.sh
	-cd api && $(MAKE)

$(BINS):
	$(CC) -o $@ $@.c $(LDFLAGS) $(CFLAGS)
	./$@

reset:
	cd reset && $(MAKE)

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz a.out $(BINS)
