#!/bin/sh
rm -rf lib/sdb
git clone ../../../ lib/sdb
make -C lib/sdb src/sdb_version.h
rm -rf lib/sdb/.git*
npm publish
