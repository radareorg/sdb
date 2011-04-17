#!/bin/sh
sdb=../src/sdb
md5=md5

if [ -z "$1" ]; then
	echo "Usage: loginout.sh [login]"
	exit 1
fi
login=$1
hash=`$sdb auth.sdb auth.$login`
if [ -z "$hash" ]; then
	echo "Unknown user"
	exit 1
fi
$sdb auth.sdb cookie.$login=
