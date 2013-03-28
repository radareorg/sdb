DESTDIR?=
PREFIX?=/usr

VERSION=0.6

CFLAGS+=-DVERSION=\"${VERSION}\"

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
