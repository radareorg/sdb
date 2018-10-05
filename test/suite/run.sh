#!/bin/sh

TOTAL=0
SUCCESS=0
FAILED=0
FIXED=0
BROKEN=0

if [ "$1" = wine ]; then
	SDB="wine `dirname $0`/../../src/sdb.exe"
	WINEMODE=1
else
	WINEMODE=0
	SDB_TEST_PATH=`dirname "$0"`/..
	. "${SDB_TEST_PATH}"/sdb-test.sh "${SDB_TEST_PATH}"
fi


fail() {
	if [ -n "$1" ]; then
		rm -f $1
		echo " ${RED}ERROR${RESET} - $2" >/dev/stderr
	fi
	FAILED=$((${FAILED}+1))
}

success() {
	if [ -n "$1" ]; then
		rm -f $1
		echo "   ${GREEN}OK${RESET}  - $2" >/dev/stderr
	fi
	SUCCESS=$((${SUCCESS}+1))
}

GREEN=`printf "\033[32m"`
YELLOW=`printf "\033[33m"`
BLUE=`printf "\033[34m"`
LILA=`printf "\033[35m"`
RED=`printf "\033[31m"`
RESET=`printf "\033[0m"`
BRKMOD=0
WINRAR=0

brk() {
	BRKMOD=1
	run $@
	BRKMOD=0
}

run2() {
	WINRAR=1
	run $@
	WINRAR=0
}

run() {
	K="$1"
	if [ 1 = "${WINRAR}" ]; then
		A=$(${SDB} a "$K";${SDB} a "$2";rm -f a)
		B=$(printf -- "$3\n")
	else
		A=$(${SDB} - "$K")
		B=$(printf -- "$2\n")
	fi
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
		#echo "            A($A)"
		#echo "            B($B)"
	fi
	if [ "$A" = "$B" ]; then
		if [ 1 = "${BRKMOD}" ]; then
			echo " ${YELLOW}FIXED${RESET} - "`printf -- "$B"`" = $K"  | tr '\n' ' '
			FIXED=$((${FIXED}+1))
		else
			echo "   ${GREEN}OK${RESET}  - "`printf -- "$B"`" = $K"  | tr '\n' ' '
		fi
		success >/dev/null
	else
		if [ 1 = "${BRKMOD}" ]; then
			echo " ${LILA}BROKEN${RESET} - "`printf -- "$B"`" = "\
			"$K  =>  "`printf -- "$A"` | tr '\n' ' '
			BROKEN=$((${BROKEN}+1))
			FAILED=$((${FAILED}-1))
		else
			echo " ${RED}ERROR${RESET} - "`printf -- "$B"`" = "\
			"$K  =>  "`printf -- "$A"` | tr '\n' ' '
		fi
		fail >/dev/null
	fi
	echo
}

test_create() {
	NAME="create"
	rm -f .a
	$SDB .a = <<EOF
a=c
b=d
EOF
	A="`$SDB .a a`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "c" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_create2() {
	NAME="create2"
	rm -f .a
	$SDB .a = <<EOF
a=c
b=d
EOF
	A="`$SDB .a b`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "d" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_create3() {
	NAME="create3"
	rm -f .a
	$SDB .a = <<EOF
another=world
this=isakey
EOF
	A="`$SDB .a`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "`printf 'another=world\nthis=isakey'`" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_store() {
	NAME="store"
	rm -f .a
	$SDB .a a=c
	A="`$SDB .a a`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "c" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_restore() {
	NAME="restore"
	rm -f .a
	$SDB .a a=a
	$SDB .a a=c
	A="`$SDB .a a`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "c" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_store2() {
	NAME="store2"
	rm -f .a
	$SDB .a a=c a=b
	if [ "`$SDB .a |wc -l |awk '{print $1}'`" = 1 ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_remove() {
	NAME="remove"
	rm -f .a
	$SDB .a alpha=beta teta=omega al=pha "~alp" omega=upsilon
	$SDB .a "~omega"
	$SDB .a "teta="

	A="`$SDB .a`"
	if [ "${WINEMODE}" = 1 ]; then
		A="`echo "$A" | perl -0 -pe 's/\r//g;s/\n\Z//'`"
	fi
	if [ "$A" = "al=pha" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

title() {
	echo "------------------------------------------------"
	printf "|                                              |\r"
	echo "| $1 |"
	echo "------------------------------------------------"
}

title "Strings"
run2 "K=V;K=" "K" ''
run "K=V;K;V" V
run "K=V;+K=Y;K" VY
run "K=V;K=Y;K" Y
run "K=Hello;K" Hello
run "K=V;+K= Y;K" 'V Y'
run "K=AB;-K=B;K" "A"

title "References"
run 'K=V;V=3;$K' 3
run 'K=V;V=$K;V' V
run 'K=V;$K=$K;V' V
run 'K=V;Y=$K;Y' V
run 'K=V;$K=$K;$K' V
run 'V=$K;V' ''

title "Matching"
run "KAI=foo;KAE=bar;~KA;*" ''
run "KAI=foo;KEE=bar;~KA;*" 'KEE=bar'

title "Deletion"
run "FOO=bar;FOO=;*" ''
run "FOO=bar;~bar;*" 'FOO=bar'
run 'FOO=bar;g={"foo":1,"bar":{"cow":3}};~g;*' 'FOO=bar'
run 'FOO=bar;~g={"foo":1,"bar":{"cow":3}};*' 'FOO=bar'

title "Numbers"
run "K=0;+K" 1
run "K=1;-K" 0
run "K=0;-K" 0
run "K=0;+K=1;K" 1
run "K=0;+K=2;K" 2
run "K=10;-K=4;K" 6
run "K=0;-K=10;K" 0
run "K=-1;+K" 1
run "K=-2;+K" 1
run "K=0;+-+-+K;K" "0x1\n0"
run "K=18446744073709551615;+K;" 0
run "K=2;+K=-1;K" 1
run "K=2;+K=+1;K" 3
run "K=2;-K=+1;K" 1
run "K=2;-K=-1;K" 3
run "-0=-0;0" 0

title "Hash"
run "#a" "0x0002b5c4"
run "#a;#b" "0x0002b5c4

0x0002b5c7

"
title "Arrays"
run "[]K=1,2,3;[2]K=a,b;[2]K" a
run "[]K=1,2,3;[?]K" 3
run "K=1,2,3;[1]K=9;[]K" "1\n9\n3"
run "[]K=1,2,3;[1]K" 2
run "K=1,2,3;[-]K;[]K" "3\n1\n2"
run "[]K=1,2,3;[+]K;[]K" "1\n2\n3"
run "K=1,2,3;[-1]K" "2"
run "[]K=1,2,3;[-1]K;[?]K" "2\n2"
run "K=1,2,3;[-1]K=;[]K" "2\n1\n3"
run "[]K=1,2,3;[+1]K=a;[]K" "1\na\n2\n3"
run "K=1,2,3;[0]K" 1
run "[]K=1,2,3;[4]K" ''
run "K=1;[1]K=2;K" '1,2'
run "[]K=1,2;[+]K=3;[]K" '1\n2\n3'
run "[]K=1,2;[+]K=2;[]K" '1\n2'
run "K=a,b,c;[-b]K;K" "a,c"
# XXX run "[]K=a,b,c;[b]K" "1"
run "[]K=a,b,c;[-]K=b;[]K" "a\nc"
run "[b]b" "" # crash test

run "foo=1,2,3,4;+[1]foo=1;foo" "1,0x3,3,4"
run "foo=1,2,3,4;+[1]foo" "3"
run "foo=1,2,3,4;-[1]foo" "1"

title "Sorted arrays"
run "[]K=cd,bc,ab;[!]K;K" "ab,bc,cd"
run "[]K=aa,bb,yy;[!+]K=xx,qq,aa;K" "aa,aa,bb,qq,xx,yy"
run "[!+]K=xx,qq,aa;[!+]K=ff,bb,zz;K" "aa,bb,ff,qq,xx,zz"
run "[]K=,x,a, ;[!]K;K" ", ,a,x"
run "[]K=9,9,8,5,1,10;[#]K;K" "1,5,8,9,9,10"
run "[]K=0x1,0x5,0xf;[#+]K=0xa;K" "0x1,0x5,0xa,0xf"
run "[#+]K=0x1;[#+]K=0xa;[#+]K=0x5;K" "0x1,0x5,0xa"

title "Set"
run "K=;[+]K=1;[+]K=1;K" 1
run "K=;[+]K=a;[+]K=b;K" "a,b"
run "K=;[+]K=a;[+]K=b;[+]K=a;K" "a,b"
run "K=;[+]K=a;[+]K=b;[-]K=a;K" "b"
run "K=;[+]K=a;[+]K=b;[-]K=a;[-]K=b;K" ""

title "Stack"
run "K=1,2;[++]K=;K" ',1,2'
run "K=1,2;[--]K=;K" '1,2'
run "K=1,2;[--]K;K" '1\n2'
run "K=;[++]K=1;K" 1
run "[]K=;[++]K=1;K" 1
run "[]K=;[++]K=1;[++]K=2;K" "2,1"
run "K=1,2,3;[--]K;[?]K" "1\n2"
run "K=1,2,3;[--]K;[]K" "1\n2\n3"

# [+] and [-] is wrongly defined. mixes stack and set concepts
#title "Wrong parsing "
#run "[]K=1,2,3;[-]K;[+]K=4;[]K" "3\n1\n2\n4"
#run "[]K=1,2,3\n[-]K\n[+]K=4\n[]K" "3\n1\n2\n4"

title "Negative"
run "a=-2;a" -2
run "a=-3;+a" 1
run "a=-2;+a=1;a" 1
run "a=-2;+a=4;a" 4
run "a=0;-a" 0
run "a=0;-a=4;a" 0
run "a=0;+a=-4;a" 0
run "a=1;+a=-4;a" 0
run "a=0;-a=4;a" 0
run "a=-4;+a=1;a" 1
run "a=-4;-a=1;a" 0
run "a=-4;+a=-1;a" 0

title "Quoted strings"
run "c=3;a=\"b;c\";a" "b;c"
run "c=3;a=\"b\\\"c\";a" "b\"c"

title "Types"
run "c=-1;?c" "number"
run "c=1;?c" "number"
run "c=0;?c" "number"
run "c=a;?c" "string"
run "c=true;?c" "boolean"
run 'c={"foo":1,"bar":[1,2,3]};?c' "json"
run 'c=;?c' 'undefined'

title "JSON"
# indent is broken because run() doesnt handle multiline outputs, but it works :P
run 'foo={"bar":[1,2,3]};foo:' '{\n  "bar": [\n    1,\n    2,\n    3\n  ]\n}\n'
brk 'foo={"bar":[1,2,3]};foo:bar[1]=;foo' '{"bar":[1,3]}'
brk 'foo={"bar":[1,2,3,4]};foo:bar[0]=;foo' '{"bar":[2,3,4]}'
brk 'foo={"bar":[1,2,3,4]};foo:bar[1]=;foo' '{"bar":[1,3,4]}'
brk 'foo={"bar":[1,2,3,4]};foo:bar[2]=;foo' '{"bar":[1,2,4]}'
brk 'foo={"bar":[1,2,3,4]};foo:bar[3]=;foo' '{"bar":[1,2,3]}'
run 'foo={"bar":123};foo:bar=pop;foo:bar=cow;foo:bar' cow
run 'foo={"bar":123};foo:bar=3;foo:bar' 3
run 'foo={"bar":123};foo:bar=pop;foo:bar' 'pop'
run 'foo={"bar":123};foo:bar=true;foo:bar' 'true'
run 'foo=[1,2,3];foo:[1]' 2
run 'foo=[1,2,3];+foo:[1];foo:[1]' "3\n3"
run 'foo=[1,2,3];foo:[1]=999;foo' '[1,999,3]'
run 'foo={"bar":"V"};foo:bar' V
run 'a={"a":1,"b":2};a:a=;a:b=;a' '{}'
run 'a={"a":1,"b":2};a:b=;a' '{"a":1}'
run 'foo={"bar":123};foo:bar' 123
run 'foo={"bar":123};foo:bar=69;foo:bar' 69
run 'foo={"bar":"pop"};foo:bar="jiji";foo:bar' jiji
run 'foo={"bar":[1,2]};foo:bar[0]' 1

run 'foo={"bar":"pop"};foo:bar="jiji";foo' '{"bar":"jiji"}'
run 'foo={"pop":123,"bar":"cow"};foo:pop=;foo' '{"bar":"cow"}'
run 'foo={"pop":123,"bar":"cow"};foo:pop=;foo:pop=123;foo' '{"pop":123,"bar":"cow"}'
run 'foo={};foo:pop=123;foo' '{"pop":123}'
run 'foo=;foo:pop=123;foo' '{"pop":123}'

title "Limits"
run "a=0x8000000000000001;a" 0x8000000000000001
# use hex for big numbers only? what is a big number
run "a=0x8000000000000001;+a" 0x8000000000000002
# 254
run "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=123;AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" 123
# 255
run "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=123;AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" ''
# 255 // MUST FAIL
run2 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=123" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" ''
# 254 // MUST BE OK
run2 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=123" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" 123
# 253 // MUST BE OK
run "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=123;AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" 123

title "Slurp"
printf "K=V\nK\n" > .t
run "..t" V
run "..f" '' 2>/dev/null
run "..t;..t" "V\n\nV\n\n"
rm -f .t .f

title "Base64"
run "%a=Hello;a" SGVsbG8=
run "%a=Hello;%a" Hello
run "%a=bar;%a" bar
run "%a=bu;%a" bu
run "%a=;%a" ""
run "a=1,2,3;%[1]a=WIN;%[1]a" WIN
run "a=1,2,3;%[1]a=WIN;[1]a" V0lO
run "a=1,2,3;%[1]a=WIN;%[1]a" WIN

title "Namespaces"
run "a/a=3;*" ""
run "a/a=3;a/*" "a=3"
run "a/a=3;a/a" "3"
run "a/b/c=3;a/c/b=4;a/**" "b\nc"
run "a/b/c=3;a/c/b=4;***" "a/b/c=3\na/c/b=4"
run "foo/bar/cow=3;foo/plop=2;foo/ra/re=4;foo/bar/meuh=1;***" "foo/plop=2\nfoo/bar/cow=3\nfoo/bar/meuh=1\nfoo/ra/re=4"
# TODO: How to delete a namespace? do we want to permit this?

title "Shell"
test_create
test_create2
test_create3
test_store
test_store2
test_restore
test_remove

title "Results"

TOTAL=$((${SUCCESS}+${FAILED}))
#RATIO=`echo "100 ${FAILED} * ${TOTAL} / n" | dc`
RATIO=$(((100*${FAILED})/${TOTAL}))
echo "  TOTAL       ${TOTAL}"   > /dev/stderr
echo "  SUCCESS     ${SUCCESS}" > /dev/stderr
echo "  FAILED      ${FAILED}"  > /dev/stderr
echo "  FIXED       ${FIXED}"  > /dev/stderr
echo "  BROKEN      ${BROKEN}"  > /dev/stderr
echo "  BROKENNESS  ${RATIO}%"  > /dev/stderr

if [ "${FAILED}" = 0 ]; then
	exit 0
else
	exit 1
fi
