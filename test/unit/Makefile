OBJECTS = $(patsubst %.c,%,$(wildcard *.c))
LDFLAGS += $(shell pkg-config --libs sdb)
CFLAGS += $(shell pkg-config --cflags sdb) -g

all: $(OBJECTS)

run: all
	./run.sh

$(OBJECTS):%:%.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

.PHONY: all run $(OBJECTS)
