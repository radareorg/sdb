#!/bin/sh
# 1Milion = 15s
# 100000 = 1.3s
# 10000 = 0.3s

sdb=../src/sdb
SIZE=10000
SIZE=100000
#SIZE=1000
SIZE=30000

makekeys() {
	 i=0
	 while : ; do
		 echo "key$i=testdata$i"
		 i=$(($i+1))
		 [ $i = $SIZE ] && break
	 done
}

echo "[**] Generating $SIZE keyvalues..."
makekeys > a
echo "[**] Verifying keys..."
NUM=$((0+`wc -l< a`))
if [ $NUM != $SIZE ]; then
	echo "Failed at generating the keyvalues"
	exit 1
fi
# TODO test if count matches
rm -f test.db

echo "[**] Bundling = database of ${SIZE} keyvalues..."
time cat a | $sdb test.db =

echo "[**] Counting keys..."
time $sdb test.db | wc -l | tee test.count
COUNT=$((0+`cat test.count`))
if [ ${COUNT} -ne ${SIZE} ]; then
	echo "Database storage is wrong: $SIZE vs ${COUNT}"
	rm -f test.count
	exit 1
fi

echo "[**] Creating - database of ${SIZE} keyvalues..."
rm -f test.db
time cat a | $sdb test.db -

echo "[**] Updating - database of ${SIZE} keyvalues..."
time cat a | $sdb test.db -

printf "[**] Database size: "
du -hs test.db

echo "[**] Updating database with ${SIZE} keyvalues..."
time cat a | $sdb test.db -

#echo "[**] Updating database using stdin..."
#time cat a | $sdb test.db -

echo "[**] Fetching a single key..."
time $sdb test.db key999

echo "[**] Counting stored keyvalues..."
time $sdb test.db | wc -l | tee test.count
COUNT=$((0+`cat test.count`))
if [ ${COUNT} -ne ${SIZE} ]; then
	echo "Database storage is wrong: $SIZE vs ${COUNT}"
	rm -f test.count
	exit 1
fi
rm -f test.count
exit 0
