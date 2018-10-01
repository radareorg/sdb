OBJECTS = $(patsubst %.c,%,$(wildcard *.c))
SRCDIR=../../src
BASEDIR?=${SRCDIR}
LDFLAGS += -static -L${BASEDIR} -lsdb
CFLAGS += -I${SRCDIR} -I${BASEDIR}
CFLAGS += -g -O0
# CFLAGS += -fsanitize=address

all: $(OBJECTS)

r run: all
	./run.sh

$(OBJECTS):%:%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

.PHONY: all run $(OBJECTS)
