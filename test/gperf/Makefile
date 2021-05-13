SDB=../../src/sdb

all: plain bin

plain:
	$(SDB) -G data.sdb.txt > data.c

bin:
	$(SDB) data.sdb = < data.sdb.txt
	$(SDB) -G data.sdb > data.gperf
	gperf -aclEDCIG --null-strings -H sdb_hash_c_data -N sdb_get_c_data -t data.gperf > data.c
	gcc -DMAIN=1 data.c
	./a.out

clean:
	rm -f data.gperf data.c data.h
