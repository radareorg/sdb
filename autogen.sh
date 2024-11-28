#!/bin/sh
V=$(grep ^SDBVER config.mk | cut -d = -f2) 
meson rewrite kwargs set project / version "$V"
rm -f include/sdb/version.h
make include/sdb/version.h
