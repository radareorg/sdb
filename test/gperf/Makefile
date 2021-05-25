SDB=../../src/sdb

all: plain text bin

plain:
	$(SDB) -G data.sdb.txt > data.c

text:
	$(SDB) -t -G data.sdb.txt > data-text.c
	gcc -DMAIN=1 -o b.out data-text.c
	./b.out

bin:
	$(SDB) data.sdb = < data.sdb.txt
	$(SDB) -G data.sdb > data.gperf
	gperf -aclEDCIG --null-strings -H sdb_hash_c_data -N sdb_get_c_data -t data.gperf > data.c
	gcc -DMAIN=1 data.c
	./a.out | grep "data_bar 4"

clean:
	rm -f data.gperf data.c data.h
