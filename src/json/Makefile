BIN=a.out
OBJ=rangstr.o json.o js0n.o sdb.o main.o

all: ${BIN}

${BIN}: ${OBJ}
	${CC} -o ${BIN} ${OBJ}

clean:
	rm -f ${OBJ} ${BIN}

mrproper: clean
	rm -rf js0n

test:
	./${BIN} glossary.GlossDiv.GlossList.GlossEntry.GlossDef.GlossSeeAlso[0] < test.json

update:
	rm -rf js0n js0n.c
	${MAKE} js0n.c

js0n.c:
	git clone git://github.com/quartzjer/js0n.git
	cp js0n/js0n.c js0n.c
	rm -rf js0n
