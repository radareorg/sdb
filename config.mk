DESTDIR?=
PREFIX?=/usr

CFLAGS+=-Wall
#CFLAGS+=-O3
CFLAGS+=-ggdb -g -Wall -O0

HAVE_VALA=$(shell valac --version)
