include ../sdb-test.mk

all: array refs
	./array
	./refs

array: array.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS) $(SDB_CFLAGS) $(SDB_LDFLAGS)

refs: refs.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS) $(SDB_CFLAGS) $(SDB_LDFLAGS)

clean:
	rm -f ./array ./refs
