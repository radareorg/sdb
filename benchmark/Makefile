HAVE_VALA=$(shell valac --version)
HAVE_LUM=$(shell lum -h)
HAVE_NEWLISP=$(shell newlisp -h)
HAVE_LUVIT=$(shell luvit -h)
HAVE_NODE=$(shell node -h)

CFLAGS+=`pkg-config --cflags sdb`
LDFLAGS+=`pkg-config --libs sdb`
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
	@printf ".sdb:"
	@(time node node-sdb.js) 2>&1 | grep real | sed s,real,,
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
ifneq (${HAVE_LUM},)
	@if [ ! -d modules/sdb ]; then lum -i sdb ; fi
	@printf "..sdb:"
	@(time luvit luvit-sdb.lua) 2>&1 | grep real | sed s,real,,
endif

clean:
	rm -rf a.out test _lumwrk

mrproper: clean
	rm -rf modules
