#!/bin/sh

TOTAL=0
SUCCESS=0
FAILED=0

SDB=`dirname $0`/../../src/sdb

if [ ! -x $SDB ]; then
	echo "Cannot find ${SDB}"
	exit 1
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
RED=`printf "\033[31m"`
RESET=`printf "\033[0m"`

run() {
	K="$1"
	#A="`echo "$K" | $SDB -`"
	A="`${SDB} - \"$K\"`"
	#A="`printf -- "$K" | $SDB -`"
	#A="`printf -- \"$K\" | $SDB -`"
	B="`printf -- \"$2\n\"`"
	if [ "$A" = "$B" ]; then
		echo "   ${GREEN}OK${RESET}  - "`printf -- "$B"`" = $K"  | tr '\n' ' '
		success >/dev/null
	else
		echo " ${RED}ERROR${RESET} - "`printf -- "$B"`" = "\
			"$K  =>  "`printf -- "$A"` | tr '\n' ' '
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
	if [ "`$SDB .a a`" = "c" ]; then
		success .a $NAME
	else
		fail .a $NAME
	fi
}

test_store() {
	NAME="store"
	rm -f .a
	$SDB .a a=c
	if [ "`$SDB .a a`" = "c" ]; then
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
	if [ "`$SDB .a a`" = "c" ]; then
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

title() {
	echo "------------------------------------------------"
	printf "|                                              |\r"
	echo "| $1 |"
	echo "------------------------------------------------"
}

title "Strings"
run "K=V;K;V" V
run "K=V;+K=Y;K" VY
run "K=V;K=Y;K" Y
run "K=Hello;K" Hello
run "K=V;+K= Y;K" 'V Y'

title "References"
run 'K=V;V=3;$K' 3
run 'K=V;V=$K;V' V
run 'K=V;$K=$K;V' V
run 'K=V;Y=$K;Y' V
run 'K=V;$K=$K;$K' V
run 'V=$K;V' ''

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

title "Arrays"
run "[]K=1,2,3;[2]K=a,b;[2]K" a
run "[]K=1,2,3;[?]K" 3
run "[]K=1,2,3;[1]K=9;[]K" "1\n9\n3"
run "[]K=1,2,3;[1]K" 2
run "[]K=1,2,3;[-]K;[]K" "3\n1\n2"
run "[]K=1,2,3;[+]K;[]K" "1\n2\n3"
run "[]K=1,2,3;[-1]K" "2"
run "[]K=1,2,3;[-1]K;[?]K" "2\n2"
run "[]K=1,2,3;[-1]K=;[]K" "2\n1\n3"
run "[]K=1,2,3;[+1]K=a;[]K" "1\na\n2\n3"
run "[]K=1,2,3;[0]K" 1
run "[]K=1,2,3;[4]K" ''
run "[]K=1;[1]K=2;K" '1,2'
run "[]K=1,2;[+]K=3;[]K" '1\n2\n3'
run "[]K=a,b,c;[-b]K;K" "a,c"
# XXX run "[]K=a,b,c;[b]K" "1"
run "[]K=a,b,c;[-]K=b;[]K" "a\nc"
run "[b]b" "" # crash test

title "Stack"
run "[]K=;[+]K=1;K" 1
run "[]K=;[+]K=1;[+]K=2;[]K" "1\n2"
run "[]K=1,2,3;[-]K;[?]K" "3\n2"
run "[]K=1,2,3;[-]K;[]K" "3\n1\n2" # XXX

title "Wrong parsing "
run "[]K=1,2,3;[-]K;[+]K=4;[]K" "3\n1\n2\n4"
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

title "Quoted strings"
run "c=3;a=\"b;c\";a" "b;c"
run "c=3;a=\"b\\\"c\";a" "b\"c"

title "JSON"
run 'foo=[1,2,3];foo:[1]' 2
run 'foo=[1,2,3];+foo:[1];foo:[1]' "3\n3"
run 'foo=[1,2,3];foo:[1]=999;foo' '[1,999,3]'
run 'foo={"bar":"V"};foo:bar' V
run 'foo={"bar":123};foo:bar' 123
run 'foo={"bar":123};foo:bar=69;foo:bar' 69
run 'foo={"bar":[1,2]};foo:bar[0]' 1

title "Limits"
run "a=0x8000000000000001;a" 0x8000000000000001
# use hex for big numbers only? what is a big number
run "a=0x8000000000000001;+a" 0x8000000000000002

title "Slurp"
printf "K=V\nK\n" > .t
run "..t" V
run "..f" '' 2>/dev/null
run "..t;..t" "V\nV"
rm -f .t .f

title "Base64"
run "%a=Hello;a" SGVsbG8A
run "%a=Hello;%a" Hello
run "a=1,2,3;%[1]a=WIN;%[1]a" WIN
run "a=1,2,3;%[1]a=WIN;[1]a" V0lOAA==
run "a=1,2,3;%[1]a=WIN;%[1]a" WIN

title "Namespaces"
run "a/a=3;*" ""
run "a/a=3;a/*" "a=3"
run "a/a=3;a/a" "3"
run "a/b/c=3;a/c/b=4;a/**" "b\nc"
run "a/b/c=3;a/c/b=4;***" "a\na/b\na/c"

title "Shell"
test_create
test_store
test_store2
test_restore

title "Results"

TOTAL=$((${SUCCESS}+${FAILED}))
#RATIO=`echo "100 ${FAILED} * ${TOTAL} / n" | dc`
RATIO=$(((100*${FAILED})/${TOTAL}))
echo "  TOTAL       ${TOTAL}"   > /dev/stderr
echo "  SUCCESS     ${SUCCESS}" > /dev/stderr
echo "  FAILED      ${FAILED}"  > /dev/stderr
echo "  BROKENNESS  ${RATIO}%"  > /dev/stderr

if [ "${FAILED}" = 0 ]; then
	exit 0
else
	exit 1
fi
