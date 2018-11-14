# include ../config.mk

all:
	gcc -g -fsanitize=address btree.c $(shell pkg-config --cflags --libs r_util) main.c


clean:
	rm -rf a.out*
