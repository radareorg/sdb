LEVELDB_CFLAGS?=-I/opt/local/include/
LEVELDB_LDFLAGS?=-lleveldb -L/opt/local/lib
SDB_CFLAGS?=-I../../../src
SDB_LDFLAGS?=../../../src/libsdb.a

CC?=gcc
CXX?=g++

all: bench-sdb bench-leveldb

bench-sdb:
	${CC} -o bench-sdb bench-sdb.c $(SDB_CFLAGS) $(SDB_LDFLAGS)
	time ./bench-sdb
	PYTHONPATH=../../../bindings/python python bench-sdb.py

bench-leveldb:
	${CXX} -o bench-leveldb bench-leveldb.cxx \
		$(SDB_CFLAGS) $(SDB_LDFLAGS) \
		$(LEVELDB_CFLAGS) $(LEVELDB_LDFLAGS)
	time ./bench-leveldb
	python bench-leveldb.py

clean:
	rm -rf bench-sdb bench-leveldb a.out __*
