#!/bin/sh
./mcsdbd 2>/dev/null &
sleep 1
(
	echo a=c
	echo a
	echo a
) | ./mcsdbc > .a 2>/dev/null


FUN="`cat .a | sed -e 's, ,,g'`"
RES="`printf "c\r\nc\r"`"
echo ======
echo "$FUN" | hexdump -C
echo ======
echo "$RES" | hexdump -C
echo ======

if [ "${FUN}" = "${RES}" ]; then
	echo OK
	RET=0
else
	echo FAIL
	RET=1
fi

kill -INT %1 2>/dev/null

rm .a

exit $RET
