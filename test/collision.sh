#!/bin/sh
DB=___collide.db
SDB=../src/sdb
rm -f $DB
$SDB $DB cb=A
$SDB $DB bC=B
R=`$SDB $DB cb`
if [ "$R" == "A" ]; then
	echo "OK: collision"
	rm -f $DB
	exit 0
fi
echo "ERROR: collision"
rm -f $DB
exit 1
