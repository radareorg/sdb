# include ../config.mk

all:
	gcc -g -fsanitize=address db.c table.c $(shell pkg-config --cflags --libs r_util) t/test.c


clean:
	rm -rf a.out* sdb-db-test
