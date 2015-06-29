#!/bin/sh
DB=___delkey.db
SDB=../src/sdb
rm -f $DB
$SDB $DB foo=bar
R=`$SDB $DB foo= foo`
if [ -z "$R" ]; then
	echo "OK: delkey"
	rm -f $DB
	exit 0
fi
echo "ERROR: delkey"
rm -f $DB
exit 1
