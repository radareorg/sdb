.PHONY: all reset

BINS=fmt stack cas hook hook2 verypush
CFLAGS=-I../src
LDFLAGS=../src/libsdb.a


all: $(BINS) reset
	@suite/run.sh
	cd api && $(MAKE)

$(BINS):
	$(CC) -o $@ $@.c $(LDFLAGS) $(CFLAGS)
	./$@

reset:
	cd reset && $(MAKE)

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz a.out $(BINS)
