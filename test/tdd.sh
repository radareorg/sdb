#!/bin/sh
# test driven development tests for sdb
# author: pancake 2012-2013

tdd() {
	v="$1" ; shift ; a="`$@`"
	test "$a" = "$v"
	echo "$?  `echo $@`"
}

df="test.db"
db="../src/sdb $df"

rm -f $df
tdd 'one' $db '[]list=one,two,tri,fur
[0]list'
tdd 'two' $db '[]list=one,two,tri,fur
[1]list'
tdd '0
1' $db '[]list=1,two,tri,fur
[+0]list=0
[0]list
[1]list
'
tdd 4 $db '[]list=1,2,3,4
[?]list'

rm -f $df
$db = <<EOF
ctr=0
array=[1,2,3,4]
test={"foo":"bar"}
test2={"foo":"bar","bar":123}
test3={foo:"bar"}
test4={"foo":[1,2,3,4]}
test5={"foo":1,"bar":{"cow":3}}
test6={"foo":1,"bar":{"cow":[3,4,5]}}
test7=[{"foo":1},{"bar":2}]
EOF

# testing counters
tdd '' $db ctr=0
tdd 0 $db ctr
tdd 1 $db +ctr
tdd 2 $db +ctr
tdd 1 $db -ctr

# test 1st level array access
tdd 1 $db array?[0]
tdd 2 $db array?[1]
tdd 3 $db array?[2]
tdd '' $db array?[8]

$db array?[0]=3
tdd 3 $db array?[0]
tdd 6 $db array?[0]=6 array?[0]

#echo ===
#$db "" $db 'test?bar="cow"' 'test'
#echo ===

tdd bar $db test?foo
tdd 1 $db test4?foo[0]
tdd 99 $db test4?foo[0]=99 test4?foo[0]

tdd 3 $db test5?bar.cow
# array syntax is ignored here. should be go strict?
tdd 3 $db test5?bar.cow[0]

tdd 3 $db test6?bar.cow[0]
tdd 4 $db test6?bar.cow[1]

tdd 1 $db test7?[0].foo
tdd 2 $db test7?[1].bar
