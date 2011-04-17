DESTDIR?=
PREFIX?=/usr
PFX=${DESTDIR}${PREFIX}

all:
	cd src && ${MAKE}

vala: all
	cd vala && ${MAKE}

clean:
	cd src && ${MAKE} clean
	cd test && ${MAKE} clean
	cd vala && ${MAKE} clean

dist: clean
	cd .. && tar czvf sdb.tar.gz sdb
	cd .. && pub sdb.tar.gz

install:
	mkdir -p ${PFX}/lib ${PFX}/bin ${PFX}/include/sdb
	cp -f src/libsdb.a ${PFX}/lib
	cp -f src/sdb.h ${PFX}/include/sdb
	cp -f src/cdb.h ${PFX}/include/sdb
	cp -f src/ht.h ${PFX}/include/sdb
	cp -f src/ut32.h ${PFX}/include/sdb
	cp -f src/list.h ${PFX}/include/sdb
	cp -f src/cdb_make.h ${PFX}/include/sdb
	cp -f src/buffer.h ${PFX}/include/sdb
	cp -f src/sdb ${PFX}/bin
	cp -f vala/sdb.pc ${PFX}/lib/pkgconfig
	cp -f vala/sdb.vapi ${PFX}/share/vala/vapi

deinstall uninstall:
	rm -f ${PFX}/include/sdb.h
	rm -f ${PFX}/lib/libsdb.a

.PHONY: all vala clean dist install uninstall deinstall
