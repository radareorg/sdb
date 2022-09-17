include ../sdb-test.mk


all: array refs
	./array
	./refs

asan:
	rm -f array refs
	export asan_options=detect_leaks=0 ; \
	$(MAKE) CFLAGS="-g -fsanitize=undefined -fsanitize=address"

leaks:
	$(MAKE) CFLAGS="-g -fsanitize=undefined -fsanitize=leak -fsanitize=address"

array: array.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS) $(SDB_CFLAGS) $(SDB_LDFLAGS)

refs: refs.c
	@$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS) $(SDB_CFLAGS) $(SDB_LDFLAGS)

clean:
	rm -f ./array ./refs
