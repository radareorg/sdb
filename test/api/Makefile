include ../sdb-test.mk

all: array refs
	./array
	./refs

array: array.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

refs: refs.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f ./array ./refs
