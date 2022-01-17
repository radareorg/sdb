OBJECTS = $(patsubst %.c,%,$(wildcard *.c))

include ../sdb-test.mk

CFLAGS += -g -O0
# CFLAGS += -fsanitize=address

ASANOPTS=address undefined signed-integer-overflow leak
CFLAGS_ASAN=$(addprefix -fsanitize=,$(ASANOPTS))

all: $(OBJECTS)

r run: all
	./run.sh

asan:
	$(MAKE) CFLAGS="$(CFLAGS_ASAN)"

$(OBJECTS):%:%.c
	$(CC) $(SDB_CFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(SDB_LDFLAGS)

clean:
	rm -f $(OBJECTS)

.PHONY: all run $(OBJECTS)
