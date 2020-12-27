#!/bin/sh

# based on
# http://blog.coolaj86.com/articles/how-to-unpackage-and-repackage-pkg-osx.html

# to uninstall:
# sudo pkgutil --forget org.radare.sdb

[ -z "${MAKE}" ] && MAKE=make

SRC=/tmp/macsdbpkg
PREFIX=/usr/local
VERSION=`${MAKE} version`
DST="$(pwd)/macos-pkg/tmp"

mkdir -p "${DST}"
rm -rf "${SRC}"
(
	cd ../..
	${MAKE} mrproper 2>/dev/null
	export CFLAGS=-O2
	${MAKE} -j4 || exit 1
	# TODO: run sys/install.sh
	${MAKE} install PREFIX="${PREFIX}" DESTDIR=${SRC} || exit 1
)
if [ -d "${SRC}" ]; then
	(
		cd ${SRC} && \
		find . | cpio -o --format odc | gzip -c > "${DST}/Payload"
	)
	mkbom ${SRC} "${DST}/Bom"
	# Repackage
	pkgutil --flatten "${DST}" "${DST}/../sdb-${VERSION}.pkg"
	mv "${DST}/../sdb-${VERSION}.pkg" .
	rm -rf "${DST}"
else
	echo "Failed install. DESTDIR is empty"
	exit 1
fi

