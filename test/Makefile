all: cas hook

cas:
	${CC} -o cas cas.c -I ../src/ ../src/libsdb.a 
	./cas

hook:
	${CC} -o hook hook.c -I ../src/ ../src/libsdb.a 
	./hook

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz cas hook a.out
