all:
	@$(CC) $(CFLAGS) $(shell pkg-config --cflags --libs sdb)
	@./a.out
	@$(CC) $(CFLAGS) $(shell pkg-config --cflags --libs sdb)
	@./a.out

clean:
	rm -f a.out
