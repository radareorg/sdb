all:
	@$(CC) -o array array.c $(CFLAGS) $(shell pkg-config --cflags --libs sdb)
	@./array
	@$(CC) -o refs refs.c $(CFLAGS) $(shell pkg-config --cflags --libs sdb)
	@./refs

clean:
	rm -f a.out
