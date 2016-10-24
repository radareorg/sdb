all:
	@$(CC) $(CFLAGS) -I../../src array.c ../../src/libsdb.a
	@./a.out
	@$(CC) $(CFLAGS) -I../../src refs.c ../../src/libsdb.a
	@./a.out

clean:
	rm -f a.out
