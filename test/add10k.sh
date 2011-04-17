#!/bin/sh
# 1Milion = 15s
# 100000 = 1.3s
# 10000 = 0.3s

sdb=../src/sdb
SIZE=1000
#SIZE=1000

makekeys() {
	 i=0
	 while : ; do
		 echo "key$i=testdata$i"
		 i=$(($i+1))
		 [ $i = $SIZE ] && break
	 done
}

echo "[**] Generating keyvalues..."
makekeys > a
echo "========="
rm -f test.db
echo "[**] Creating database of ${SIZE} keyvalues..."
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
time $sdb test.db |wc -l
