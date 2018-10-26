OBJECTS = $(patsubst %.c,%,$(wildcard *.c))

include ../sdb-test.mk

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
