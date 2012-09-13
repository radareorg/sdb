HAVE_VALA=$(shell valac --version 2>/dev/null)
HAVE_LUM=$(shell lum -h 2>/dev/null)
HAVE_NEWLISP=$(shell newlisp -h 2>/dev/null)
HAVE_LUVIT=$(shell luvit -h 2>/dev/null)
HAVE_NODE=$(shell node -h)
HAVE_JAVA=$(shell javac -help 2>&1)
HAVE_PYTHON=$(shell python -h)
HAVE_PERL=$(shell perl -h)

CFLAGS+=`pkg-config --cflags sdb`
LDFLAGS+=`pkg-config --libs sdb`
#CFLAGS+=-I../src
#LDFLAGS+=-L../src -lsdb
CFLAGS+=-Wall -O3

all:
	@printf "c:"
	@${CC} -o a.out ${CFLAGS} ${LDFLAGS} t.c
	@(time ./a.out) 2>&1 | grep real | sed s,real,,
ifneq (${HAVE_NEWLISP},)
	@printf "nlisp:"
	@(time newlisp newlisp.lsp) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_NODE},)
	@printf "node:"
	@(time node node.js) 2>&1 | grep real | sed s,real,,
	@printf "n.ffi:"
	@(time node node-ffi.js) 2>&1 | grep real | sed s,real,,
	@printf "n.cc:"
	@(time node node-cc.js) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_VALA},)
	@printf "vala:"
	@-valac --pkg sdb test.vala
	@(time ./test) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_LUVIT},)
	@printf "luvit:"
	@(time luvit luvit.lua) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_JAVA},)
	@printf "java:"
	@if [ ! -e json.jar ] ; then wget lolcathost.org/b/json.jar ; else true ; fi
	@javac -cp json.jar java.java
	@(time java -cp json.jar:. java) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_PYTHON},)
	@printf "python:"
	@(time python python.py) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_PERL},)
	@printf "perl:"
	@# sudo cpan -i JSON
	@(time perl perl.pl) 2>&1 | grep real | sed s,real,,
endif
ifneq (${HAVE_LUM},)
	@if [ ! -d modules/sdb ]; then lum -i sdb ; fi
	@printf "l.sdb:"
	@(time luvit luvit-sdb.lua) 2>&1 | grep real | sed s,real,,
endif

clean:
	rm -rf a.out test _lumwrk

mrproper: clean
	rm -rf modules
