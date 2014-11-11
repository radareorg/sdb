include ../config.mk

FILES=sdb mcsdb

all:
	for a in ${FILES} ; do \
		sed -e 's,@@VERSION@@,${SDBVER},g' \
			-e 's,@@PREFIX@@,${PREFIX},g' \
			< $$a.d > $$a.pc ; \
	done
