all:
	@$(CC) -I../../src array.c ../../src/libsdb.a
	@./a.out
	@$(CC) -I../../src refs.c ../../src/libsdb.a
	@./a.out

clean:
	rm -f a.out
