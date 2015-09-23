#!/bin/sh
rm -rf node_modules
npm install
rm -rf lib/sdb
git clone ../../../ lib/sdb
make -C lib/sdb src/sdb_version.h
rm -rf lib/sdb/.git*
npm publish
