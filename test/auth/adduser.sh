#!/bin/sh
sdb=../src/sdb

if [ -z "$1" ]; then
	echo "Usage: adduser.sh [username]"
	exit 1
fi
usr=$1
if [ -n "$sdb auth.sdb read.$usr" ]; then
	echo "User already exists. Use passwd.sh to reset password"
	exit 1
fi

echo "Space separated databases with READ access?"
read r
$sdb read.pancake=$r

echo "Space separated databases with WRITE access?"
read w
$sdb write.pancake=$w

sh passwd.sh $usr
