OBJECTS = $(patsubst %.c,%,$(wildcard *.c))
LDFLAGS += ../../src/libsdb.a
#$(shell pkg-config --libs sdb)
CFLAGS += -I../../src
#$(shell pkg-config --cflags sdb) -g -O0

all: $(OBJECTS)

run: all
	./run.sh

$(OBJECTS):%:%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

.PHONY: all run $(OBJECTS)
