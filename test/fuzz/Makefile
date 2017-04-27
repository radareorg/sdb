FILES=$(shell find . -type f | grep id:)
SDB=../../src/sdb

all:
	for a in $(FILES) ; do \
		echo $$a ; \
		valgrind $(SDB) - < $$a ; \
	done
	
