include config.mk
PFX=${DESTDIR}${PREFIX}

all:
	cd src && ${MAKE}
	cd memcache && ${MAKE}

vala: all
	cd vala && ${MAKE}

clean:
	cd src && ${MAKE} clean
	cd memcache && ${MAKE} clean
	cd test && ${MAKE} clean
	cd vala && ${MAKE} clean

dist: clean
	cd .. && tar czvf sdb.tar.gz `find sdb| grep -v \.hg`
	cd .. && pub sdb.tar.gz

install:
	mkdir -p ${PFX}/lib/pkgconfig ${PFX}/bin 
	mkdir -p ${PFX}/share/vala/vapi ${PFX}/include/sdb
	cp -f src/libsdb.a ${PFX}/lib
	cp -f src/sdb.h ${PFX}/include/sdb
	cp -f src/cdb.h ${PFX}/include/sdb
	cp -f src/ht.h ${PFX}/include/sdb
	cp -f src/types.h ${PFX}/include/sdb
	cp -f src/list.h ${PFX}/include/sdb
	cp -f src/cdb_make.h ${PFX}/include/sdb
	cp -f src/buffer.h ${PFX}/include/sdb
	cp -f src/config.h ${PFX}/include/sdb
	cp -f src/sdb ${PFX}/bin
	cp -f memcache/libmcsdb.a ${PFX}/lib
	cp -f memcache/mcsdb.h ${PFX}/include/sdb
	cp -f memcache/mcsdbd ${PFX}/bin
	cp -f memcache/mcsdbc ${PFX}/bin
	cp -f vala/sdb.pc ${PFX}/lib/pkgconfig
	cp -f vala/sdb.vapi ${PFX}/share/vala/vapi
	cp -f vala/mcsdb.pc ${PFX}/lib/pkgconfig
	cp -f vala/mcsdb.vapi ${PFX}/share/vala/vapi

deinstall uninstall:
	rm -f ${PFX}/include/sdb.h
	rm -f ${PFX}/lib/libsdb.a

.PHONY: all vala clean dist install uninstall deinstall
