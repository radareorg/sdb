MAKEFILE_PATH:=$(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR:=$(abspath $(patsubst %/,%,$(dir $(MAKEFILE_PATH))))

SRCDIR=${CURRENT_DIR}/../src
BASEDIR?=${SRCDIR}
CFLAGS+=-I${SRCDIR} -I${BASEDIR} ${USER_CFLAGS}
LDFLAGS+=-static -L${BASEDIR} -lsdb ${USER_LDFLAGS}
SDB=${BASEDIR}/sdb
