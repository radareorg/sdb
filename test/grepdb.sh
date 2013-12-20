#!/bin/sh
# toy implementation of key-value in shellscript
db() {
	ARG="$1"
	if [ -z "$ARG" ]; then
		[ $n != 0 ] && return 1
		grep = "$FILE"
	elif [ -z "`echo \"$ARG\" | grep =`" ]; then
		grep "^${ARG}=" "$FILE" | cut -d = -f 2-
	else
		KEY="`echo $ARG|cut -d = -f 1`"
		VAL="`echo $ARG| cut -d = -f 2`"
		if [ -z "$VAL" ]; then
			grep -v "^$KEY=" "$FILE" > ".$FILE"
			mv ".$FILE" "$FILE"
		else
			grep -v "^$KEY=" "$FILE" > ".$FILE"
			echo "$ARG" >> ".$FILE"
			mv ".$FILE" "$FILE"
		fi
	fi
	return 0
}

if [ -z "$1" ]; then
	echo "Usage: sdb.sh file [key[=value]]"
	exit 1
fi
FILE="$1"
if [ "$FILE" = - ]; then
	FILE=_
	if [ -z "$2" ]; then
		while : ; do
			read A
			db "$A"
		done
	fi
fi
touch "$FILE"

n=0
while : ; do
	ARG="$2"
	db "$2"
	[ $? = 1 ] && break
	shift
	n=$(($N+1))
done
[ "$FILE" = - ] && rm -f "$FILE"
