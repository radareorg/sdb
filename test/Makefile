all: cas

cas:
	${CC} cas.c -I ../src/ ../src/libsdb.a 
	./a.out

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz
