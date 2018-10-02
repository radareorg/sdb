#!/bin/sh
# 1Milion = 15s
# 100000 = 1.3s
# 10000 = 0.3s

. ./sdb-test.sh

SIZE=10000
SIZE=1000000
#SIZE=1000
SIZE=80000

if [ -n "$1" ]; then
	SIZE="$1"
fi

makekeys() {
	 i=0
	 while : ; do
		 echo "key$i=testdata$i"
		 i=$(($i+1))
		 [ $i = $SIZE ] && break
	 done
}

msg() {
	echo "\033[33m$@\033[0m"
}

err() {
	echo "\033[31m$@\033[0m"
}

ok() {
	echo "\033[32m$@\033[0m"
}

msg "[**] Generating $SIZE keyvalues..."
makekeys > a
msg "[**] Verifying keys..."
NUM=$((0+`wc -l< a`))
if [ $NUM != $SIZE ]; then
	err "Failed at generating the keyvalues"
	exit 1
fi
# TODO test if count matches
rm -f test.db

msg "[**] Bundling = database of ${SIZE} keyvalues..."
time cat a | $SDB test.db =

msg "[**] Counting keys..."
time $SDB test.db | wc -l | tee test.count
COUNT=$((0+`cat test.count`))
if [ ${COUNT} -ne ${SIZE} ]; then
	err "Database storage is wrong: $SIZE vs ${COUNT}"
	rm -f test.count
	exit 1
fi

msg "[**] Creating - database of ${SIZE} keyvalues..."
rm -f test.db
time cat a | $SDB test.db -

msg "[**] Updating - database of ${SIZE} keyvalues..."
time cat a | $SDB test.db -

printf "[**] Database size: "
du -hs test.db

msg "[**] Updating database with ${SIZE} keyvalues..."
time cat a | $SDB test.db -

#echo "[**] Updating database using stdin..."
#time cat a | $SDB test.db -

msg "[**] Fetching a single key..."
time $SDB test.db key999

msg "[**] Constructing long query... inc $INC size $SIZE"
KEYS=""
NK=0
INC=$(($SIZE/3000))
if [ "$INC" = 0 ]; then
	INC=$(($SIZE/5000))
	if [ "$INC" = 0 ]; then
		INC=$(($SIZE/100))
		if [ "$INC" = 0 ]; then
			INC=$(($SIZE/7))
			if [ "$INC" = 0 ]; then
				INC=1
			fi
		fi
	fi
fi
ok "[**] Using inc $INC for size $SIZE"
while : ; do
	k=$(($k+$INC))
	[ $k -ge $COUNT ] && break
	KEYS="$KEYS key$k "
	NK=$(($NK+1))
done

msg "[**] Many queries... "
msg "[--] Query length $(echo $KEYS | wc -c)"
msg "[--] Must be $NK"
ROWS=$(time $SDB test.db $KEYS | grep testdata | wc -l | awk '{print $1}')
if [ "$ROWS" = "$NK" ]; then
	ok "[OK] $NK rows found"
else
	err "[FAILED] $ROWS vs $NK"
	exit 1
fi

msg "[**] Counting stored keyvalues..."
time $SDB test.db | wc -l | tee test.count
COUNT=$((0+`cat test.count`))
if [ ${COUNT} -ne ${SIZE} ]; then
	err "Database storage is wrong: $SIZE vs ${COUNT}"
	rm -f test.count
	exit 1
fi
rm -f test.count
exit 0
