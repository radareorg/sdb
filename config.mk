DESTDIR?=
PREFIX?=/usr

VERSION=0.6.1

CFLAGS+=-DVERSION=\"${VERSION}\"

CFLAGS_STD?=-D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700
CFLAGS+=${CFLAGS_STD}

CFLAGS+=-Wall
#CFLAGS+=-O3
#CFLAGS+=-ggdb -g -Wall -O0
CFLAGS_SHARED?=-fPIC -shared -fvisibility=hidden

HAVE_VALA=$(shell valac --version)
# This is hacky
OS=$(shell uname)
ARCH=$(shell uname -m)
ifeq (${OS},Darwin)
SOEXT=dylib
LDFLAGS+=-dynamic
LDFLAGS_SHARED?=-fPIC -shared
 ifeq (${ARCH},i386)
   CC+=-arch i386 -arch x86_64
 endif
else
SOVERSION=0
SOEXT=so.0.0.0
LDFLAGS_SHARED?=-fPIC -shared
LDFLAGS_SHARED+=-Wl,-soname,libsdb.so.$(SOVERSION)
endif
RANLIB?=ranlib
EXEXT=

ifeq ($(MAKEFLAGS),s)
SILENT=1
else
SILENT=
endif
