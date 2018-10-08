#!/bin/sh

CURRENT_DIR=.
if [ "$#" -eq 1 ]; then
    CURRENT_DIR=$1
fi

SRCDIR="${CURRENT_DIR}"/../src
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
