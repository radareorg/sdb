#!/bin/sh
sdb=../src/sdb

if [ -z "$1" ]; then
	echo "Usage: passwd.sh [username]"
	exit 1
fi
usr=$1
if [ -z "$sdb auth.sdb read.$usr" ]; then
	echo "User does not exists. Use adduser.sh first"
	exit 1
fi

printf "Password: "
stty -echo
read w
stty echo
p=`printf $w | md5`
$sdb auth.sdb auth.pancake=$p
echo
