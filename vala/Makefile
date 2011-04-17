FLAGS=`PKG_CONFIG_PATH=. pkg-config --cflags --libs sdb --define-variable=libdir=../src --define-variable=includedir=../src`
GFLAGS=`pkg-config gobject-2.0 --libs --cflags`

all:
	valac -C --vapidir=. --pkg sdb test.vala
	${CC} -o test test.c ${FLAGS} ${GFLAGS}

clean:
	rm -f test a.out test.sdb test.c uuid.sdb
