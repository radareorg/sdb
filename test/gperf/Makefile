SDB=../../src/sdb

all: plain text text2 bin

plain:
	$(SDB) -G data.sdb.txt > data.c

text:
	$(SDB) -t -G data.sdb.txt > data-text.c
	gcc -DMAIN=1 -o b.out data-text.c
	./b.out | grep bar

text2:
	$(SDB) -t -C -o data-text2.c data.sdb.txt
	gcc -DMAIN=1 -o b.out data-text2.c
	./b.out | grep bar

bin:
	$(SDB) data.sdb = < data.sdb.txt
	$(SDB) -G data.sdb > data.gperf
	gperf -aclEDCIG --null-strings -H sdb_hash_c_data -N sdb_get_c_data -t data.gperf > data.c
	gcc -DMAIN=1 data.c
	./a.out | grep "data_bar 4"

clean:
	rm -f data.gperf data.c data.h
