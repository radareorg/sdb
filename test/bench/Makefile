include ../sdb-test.mk

TESTS=array new sync set stack
BINS=$(addprefix bench-,${TESTS})
OBJS=$(addsuffix .o,${BINS})

all: bench-array bench-new bench-sync bench-set bench-reset

${BINS}: ${OBJS}
	@for a in ${BINS} ; do ${CC} -o $$a $$a.o ${LDFLAGS} ; done

run test:
	for a in ${TESTS} ; do ./bench-$$a ; done

clean:
	rm -f ${BINS} ${OBJS}
