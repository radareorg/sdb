MIFILES=stats.c
MIFILES+=random.c
MIFILES+=os.c
MIFILES+=bitmap.c
MIFILES+=arena.c
MIFILES+=segment-cache.c
MIFILES+=segment.c
MIFILES+=page.c
MIFILES+=alloc.c
MIFILES+=alloc-aligned.c
MIFILES+=alloc-posix.c
MIFILES+=heap.c
MIFILES+=options.c
MIFILES+=init.c

MILF=$(addprefix mimalloc/src/,$(MIFILES))

all: mimalloc
	$(CC) -I../../include main.c $(MILF) -I mimalloc/include ../../src/libsdb.a

asan: mimalloc
	$(MAKE) -C ../.. clean ; make -C ../..  CFLAGS="-g -O0 -fsanitize=address" LDFLAGS="-fsanitize=address"; make asan2

asan2:
	$(CC) -I../../include main.c $(MILF) -I mimalloc/include ../../src/libsdb.a -fsanitize=address

mimalloc:
	git clone https://github.com/microsoft/mimalloc
