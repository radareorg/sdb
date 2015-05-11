#!/bin/sh
if [ -z "${SDB}" ]; then
	if [ -f ../src/sdb.exe ]; then
		SDB="wine ../src/sdb.exe"
	else
		SDB="../src/sdb"
	fi
fi
N="\033[0m"
R="\033[31m"
G="\033[32m"
echo "[+] Running createfail.sh..."
rm -f .newdb
${SDB} .newdb a=c
if [ "a=c" = "`sdb .newdb`" ]; then
	printf "$G  Create test     OK$N\n"
else
	printf "$R  Create test     FAIL$N\n"
	exit 1
fi
${SDB} .newdb a=d
if [ "a=d" = "`sdb .newdb`" ]; then
	printf "$G  Recreate test   OK$N\n"
else
	printf "$R  Recreate test   FAIL$N\n"
	exit 1
fi
exit 0
