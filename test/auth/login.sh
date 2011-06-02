#!/bin/sh
sdb=../src/sdb
md5=md5

if [ -z "$1" ]; then
	echo "Usage: login.sh [login]"
	exit 1
fi
login=$1
hash=`$sdb auth.sdb auth.$login`
if [ -z "$hash" ]; then
	echo "Unknown user"
	exit 1
fi
stty -echo
printf "Password: "
read pass
stty echo
myhash=`printf $pass | $md5`
echo
if [ "$hash" = "$myhash" ]; then
	cookie=`head /dev/urandom | $md5`
	$sdb auth.sdb cookie.$login=$cookie
	echo "Success. Here's your cookie: $cookie"
else
	echo "Invalid password"
fi
