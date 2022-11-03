DCC=ldc2
DFLAGS+=-boundscheck=off
#DFLAGS+=-flto=thin
#DFLAGS+=-Oz
# DCC=gcc

all:
	$(DCC) $(DFLAGS) main sdb ../../src/libsdb.a 2>&1 | (grep -v version || true)
	./main

clean:
	rm -f main main.o sdb sdb.o
