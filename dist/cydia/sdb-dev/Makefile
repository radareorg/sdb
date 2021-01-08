PACKAGE_DIR=$(shell pwd)
include ./CONFIG
DEPENDS=

all:
	${MAKE} -C ../sdb root
	sudo rm -rf control data
	${MAKE} clean
	mkdir -p data
	cp -aRf ../sdb/root/* data
	-rm -rf data/usr/bin
	-rm -rf data/usr/lib/*.dylib
	-rm -rf data/usr/lib/*.so
	-rm -rf data/usr/lib/*.so
	-rm -rf data/usr/share
	${MAKE} control
	${MAKE} deb

include ../deb_hand.mak
