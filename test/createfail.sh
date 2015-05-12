#!/bin/sh
export LANG=C
export LC_ALL=C

if [ -z "${SDB}" ]; then
#	if [ -f ../src/sdb.exe ]; then
#		SDB="wine ../src/sdb.exe"
#	else
		SDB="../src/sdb"
#	fi
fi
echo "[+] Using SDB: ${SDB}"
N="\033[0m"
R="\033[31m"
G="\033[32m"
echo "[+] Running createfail.sh..."
rm -f .newdb
${SDB} .newdb a=c
${SDB} .newdb

RET=0
WINEMODE=0
echo "$SDB" |grep -q wine && WINEMODE=1
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=c" = "$A" ]; then
	printf "$G  Create test     OK$N\n"
else
	printf "$R  Create test     FAIL$N\n"
	RET=1
fi

${SDB} .newdb a=d
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=d" = "$A" ]; then
	printf "$G  Recreate test   OK$N\n"
else
	printf "$R  Recreate test   FAIL$N\n"
	RET=1
fi
exit $RET
