.PHONY: all stack cas hook hook2

all: cas hook stack
	@suite/run.sh

stack:
	${CC} -o stack stack.c -I ../src/ ../src/libsdb.a 
	./stack

cas:
	${CC} -o cas cas.c -I ../src/ ../src/libsdb.a 
	./cas

hook:
	${CC} -o hook hook.c -I ../src/ ../src/libsdb.a 
	./hook

hook2:
	${CC} -o hook2 hook2.c -I ../src/ ../src/libsdb.a 
	./hook2

add10k:
	sh add10k.sh

clean:
	rm -f a test.db db a.gz cas hook a.out
