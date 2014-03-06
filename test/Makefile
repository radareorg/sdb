.PHONY: all

BINS=fmt stack cas hook hook2
CFLAGS=-I../src
LDFLAGS=../src/libsdb.a


all: $(BINS)
	@suite/run.sh

$(BINS):
	@$(CC) -o $@ $@.c $(LDFLAGS) $(CFLAGS)
	./$@

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz a.out $(BINS)
