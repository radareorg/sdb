include ../../config.mk

PKGCFG=PKG_CONFIG_PATH=../../pkgconfig pkg-config --cflags --libs
PKGCFG+=--define-variable=libdir=../../src --define-variable=includedir=../../src
SDB_FLAGS=$(shell ${PKGCFG} sdb)
MCPKGCFG=PKG_CONFIG_PATH=../../pkgconfig pkg-config --cflags --libs 
MCPKGCFG+=--define-variable=libdir=../../memcache --define-variable=includedir=../../memcache
MCSDB_FLAGS=$(shell ${MCPKGCFG} mcsdb) ${SDB_FLAGS}
GFLAGS=$(shell pkg-config gobject-2.0 --libs --cflags)

ifeq ($(BUILD_MEMCACHE),1)
ALL_DEPS=test mctest
else
ALL_DEPS=test
endif

all: $(ALL_DEPS)

test: test.vala
	@echo VALAC test.vala
	@valac -C --vapidir=. --pkg sdb --pkg posix test.vala
	@echo CC test.c
	@${CC} -o test test.c ${SDB_FLAGS} ${GFLAGS}

mctest: mctest.vala
	@echo VALAC mctest.vala
	@valac -C --vapidir=. --pkg mcsdb --pkg posix mctest.vala
	@echo CC mctest.c
	@${CC} -o mctest mctest.c ${MCSDB_FLAGS} ${GFLAGS}
	
clean:
	rm -f test a.out mctest.c mctest test.sdb test.c uuid.sdb
	cd types && ${MAKE} clean

.PHONY: all clean
