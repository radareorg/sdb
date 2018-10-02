#!/bin/sh
# proof that grep is faster than sdb

. ./sdb-test.sh

PATH="../src:$PATH"
printf "\033[33m[+] Creating test database...  \033[0m"
sh add10k.sh > /dev/null 2>&1
echo "DONE"

# creates test.db and 'a'
G=6561
T=a.sdb
D=test.db
K=key29988

echo
printf "\033[33m[+] System grep    \033[0m"
echo "$ grep 6561 $T"
LINES=`grep $G $T |wc -l`
echo "\033[32mLines: ${LINES}\033[0m"

echo
printf "\033[33m[+] Dump and grep  \033[0m"
echo "$ sdb $D | grep $G"
LINES=`${SDB} $D | grep $G | wc -l`
echo "\033[32mLines: ${LINES}\033[0m"

echo
printf "\033[33m[+] Internal grep  \033[0m"
echo "$ sdb -g $G $D"
LINES=`${SDB} -g $G $D | wc -l`
echo "\033[32mLines: ${LINES}\033[0m"


#########################
echo
printf "\033[33m[+] Grep get \033[0m"
echo '$ grep "^$K=" $T'
LINES="`grep "^$K=" $T |wc -l `"
echo "\033[32mLines: ${LINES}\033[0m"


echo
printf "\033[33m[+] Sdb get \033[0m"
echo "$ sdb $D $K"
LINES=`${SDB} $D $K | grep -v '^$' | wc -l`
echo "\033[32mLines: ${LINES}\033[0m"

