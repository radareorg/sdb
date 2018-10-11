#!/bin/sh
export LANG=C
export LC_ALL=C

if [ "$1" = wine ]; then
	SDB="wine ../src/sdb.exe"
fi
if [ -z "${SDB}" ]; then
	. ./sdb-test.sh
fi
WINEMODE=0
echo "$SDB" |grep -q wine && WINEMODE=1
echo "[+] Using SDB: ${SDB}"

##########################

N="\033[0m"
R="\033[31m"
G="\033[32m"
echo "[+] Running createfail.sh..."
rm -f .newdb
${SDB} .newdb a=c
${SDB} .newdb

RET=0
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

rm -f .newdb
echo a=c | ${SDB} .newdb =
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=c" = "$A" ]; then
	printf "$G  Make test       OK$N\n"
else
	printf "$R  Make test       FAIL$N\n"
	RET=1
fi


echo a=d | ${SDB} .newdb =
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=d" = "$A" ]; then
	printf "$G  Remake test     OK$N\n"
else
	printf "$R  Remake test     FAIL$N\n"
	RET=1
fi

rm -f .newdb
echo a=c | ${SDB} .newdb -
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=c" = "$A" ]; then
	printf "$G  Dash test       OK$N\n"
else
	printf "$R  Dash test       FAIL$N\n"
	RET=1
fi

echo a=d | ${SDB} .newdb -
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=d" = "$A" ]; then
	printf "$G  Redash test     OK$N\n"
else
	printf "$R  Redask test     FAIL$N\n"
	RET=1
fi

echo a=d | ${SDB} .newdb -
if [ "${WINEMODE}" = 1 ]; then
	A="`${SDB} .newdb | perl -0 -pe 's/\r//g;s/\n\Z//;'`"
else
	A="`${SDB} .newdb`"
fi
if [ "a=d" = "$A" ]; then
	printf "$G  Redash test     OK$N\n"
else
	printf "$R  Redask test     FAIL$N\n"
	RET=1
fi

exit $RET
