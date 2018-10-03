#!/bin/sh

SRCDIR=`dirname "$0"`/../src
if [ -z "${BASEDIR}" ]; then
    BASEDIR="${SRCDIR}"
fi
SDB=${BASEDIR}/sdb
if [ ! -x "${SDB}" ]; then
    SDB=$(which sdb)
fi
if [ ! -x "${SDB}" ]; then
    echo "Cannot find ${SDB}"
    exit 1
fi
