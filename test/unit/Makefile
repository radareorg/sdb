OBJECTS = $(patsubst %.c,%,$(wildcard *.c))

include ../sdb-test.mk

CFLAGS += -g -O0
# CFLAGS += -fsanitize=address

all: $(OBJECTS)

r run: all
	./run.sh

$(OBJECTS):%:%.c
	$(CC) $(SDB_CFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(SDB_LDFLAGS)

clean:
	rm -f $(OBJECTS)

.PHONY: all run $(OBJECTS)
