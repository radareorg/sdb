PACKAGE_DIR=$(shell pwd)
include ./CONFIG
DEPENDS=

all: root
	sudo rm -rf control data
	${MAKE} clean
	mkdir -p data
	cp -aRf root/* data
	rm -rf data${LIBDIR}/pkgconfig
	rm -rf data${LIBDIR}/*.a
	rm -rf data${INCDIR}
	rm -rf data/usr/share/vala
	${MAKE} deb

.PHONY: data

root:
	$(MAKE) -C ../../.. ios
	$(MAKE) -C ../../.. install PREFIX=/usr DESTDIR=/tmp/sdbtmp
	mv /tmp/sdbtmp root

include ../deb_hand.mak
