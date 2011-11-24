include config.mk
PFX=${DESTDIR}${PREFIX}
HGFILES=`find sdb-${VERSION} -type f | grep -v hg | grep -v swp`

all:
	cd src && ${MAKE}
	cd memcache && ${MAKE}
ifneq (${HAVE_VALA},)
	cd vala && ${MAKE}
	cd vala/types && ${MAKE}
endif

clean:
	cd src && ${MAKE} clean
	cd memcache && ${MAKE} clean
	cd test && ${MAKE} clean
	cd vala && ${MAKE} clean

dist: clean
	cd .. && mv sdb sdb-${VERSION} ; \
	rm -f sdb-${VERSION}.tar.gz ; \
	tar czvf sdb-${VERSION}.tar.gz ${HGFILES} ; \
	pub sdb-${VERSION}.tar.gz ; \
	mv sdb-${VERSION} sdb

install:
	mkdir -p ${PFX}/lib/pkgconfig ${PFX}/bin 
	mkdir -p ${PFX}/share/vala/vapi ${PFX}/include/sdb
	cp -f src/libsdb.a ${PFX}/lib
	cp -f src/sdb.h ${PFX}/include/sdb
	cp -f src/cdb.h ${PFX}/include/sdb
	cp -f src/ht.h ${PFX}/include/sdb
	cp -f src/types.h ${PFX}/include/sdb
	cp -f src/ls.h ${PFX}/include/sdb
	cp -f src/cdb_make.h ${PFX}/include/sdb
	cp -f src/buffer.h ${PFX}/include/sdb
	cp -f src/config.h ${PFX}/include/sdb
	cp -f src/sdb ${PFX}/bin
	cp -f memcache/libmcsdb.a ${PFX}/lib
	cp -f memcache/mcsdb.h ${PFX}/include/sdb
	cp -f memcache/mcsdbd ${PFX}/bin
	cp -f memcache/mcsdbc ${PFX}/bin
	cp -f vala/sdb.pc ${PFX}/lib/pkgconfig
	cp -f vala/mcsdb.pc ${PFX}/lib/pkgconfig
ifneq (${HAVE_VALA},)
	cp -f vala/sdb.vapi ${PFX}/share/vala/vapi
	cp -f vala/mcsdb.vapi ${PFX}/share/vala/vapi
	cd vala/types && ${MAKE} install PFX=${PFX}
endif

deinstall uninstall:
	rm -f ${PFX}/include/sdb.h
	rm -f ${PFX}/lib/libsdb.a

.PHONY: all vala clean dist install uninstall deinstall
