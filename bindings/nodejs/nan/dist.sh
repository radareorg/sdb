#!/bin/sh
rm -rf node_modules
npm install
rm -rf lib/sdb
git clone ../../../ lib/sdb
make -C lib/sdb include/sdb/version.h
rm -rf lib/sdb/.git*
npm publish
