include ../sdb-test.mk

TESTS=array new sync set reset stack
BINS=$(addprefix bench-,${TESTS})
OBJS=$(addsuffix .o,${BINS})

all: ${BINS}

%: %.c
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

run test:
	for a in ${TESTS} ; do ./bench-$$a ; done

clean:
	rm -f ${BINS} ${OBJS}
