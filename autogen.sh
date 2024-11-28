#!/bin/sh
V=$(grep ^SDBVER config.mk | cut -d = -f2) 
meson rewrite kwargs set project / version "$V"
