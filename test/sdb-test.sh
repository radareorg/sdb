#!/bin/sh

SRCDIR=`dirname "$0"`/../src
if [ -z "${BASEDIR}" ]; then
    BASEDIR="${SRCDIR}"
fi
SDB=${BASEDIR}/sdb
if [ ! -x "${SDB}" ]; then
    SDB=$(which sdb)
fi
