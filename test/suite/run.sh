#!/bin/sh

TOTAL=0
SUCCESS=0
FAILED=0

SDB=`dirname $0`/../../src/sdb

if [ ! -x $SDB ]; then
	echo "Cannot find ${SDB}"
	exit 1
fi

run() {
	K="$1"
	A=`echo "$K" | $SDB -`
	B="`echo \"$2\"`"
	GREEN="\x1b[32m"
	GREEN=`printf "\033[32m"`
	RED=`printf "\033[31m"`
	RESET=`printf "\033[0m"`
	if [ "$A" = "$B" ]; then
		echo "   ${GREEN}OK${RESET}  - "`printf -- "$B"`\
			" = "`printf -- "$K"`
		SUCCESS=$((${SUCCESS}+1))
	else
		echo " ${RED}ERROR${RESET} - "`printf -- "$B"`" = "\
			`printf -- "$K"`"  =>  "`printf -- "$A"`
		FAILED=$((${FAILED}+1))
	fi
}

title() {
	echo "------------------------------------------------"
	printf "|                                              |\r"
	echo "| $1 |"
	echo "------------------------------------------------"
}

title "Strings"
run "K=V\nK\nV" V
run "K=V\n+K=Y\nK" VY
run "K=V\nK=Y\nK" Y
run "K=Hello\nK" Hello
run "K=V\n+K= Y\nK" 'V Y'

title "References"
run 'K=V\nV=$K\nV' V
run 'K=V\n$K=$K\nV' V
run 'K=V\nY=$K\nY' V
run 'K=V\n$K=$K\n$K' V
run 'V=$K\nV' ''

title "Numbers"
run "K=0\n+K" 1
run "K=1\n-K" 0
run "K=0\n-K" 0
run "K=0\n+K=1\nK" 1
run "K=0\n+K=2\nK" 2
run "K=10\n-K=4\nK" 6
run "K=0\n-K=10\nK" 0
run "K=-1\n+K" 1
run "K=-2\n+K" 1
run "K=0\n+-+-+K" 1
run "K=18446744073709551615\n+K\n" 0

title "Arrays"
run "[]K=1,2,3\n[2]K=a,b\n[2]K" a
run "[]K=1,2,3\n[?]K" 3
run "[]K=1,2,3\n[1]K=9\n[]K" "1\n9\n3"
run "[]K=1,2,3\n[1]K" 2
run "[]K=1,2,3\n[-]K\n[]K" "3\n1\n2"
run "[]K=1,2,3\n[+]K\n[]K" "1\n2\n3"
run "[]K=1,2,3\n[-1]K" "2"
run "[]K=1,2,3\n[-1]K\n[?]K" "2\n2"
run "[]K=1,2,3\n[-1]K=\n[]K" "2\n1\n3"
run "[]K=1,2,3\n[+1]K=a\n[]K" "1\na\n2\n3"
run "[]K=1,2,3\n[0]K" 1
run "[]K=1,2,3\n[4]K" ''
run "[]K=1\n[1]K=2\nK" '1,2'
run "[]K=1,2\n[+]K=3\n[]K" '1\n2\n3'
run "[]K=a,b,c\n[-b]K\nK" "a,c"
# XXX run "[]K=a,b,c\n[b]K" "1"
run "[]K=a,b,c\n[-]K=b\n[]K" "a\nc"
run "[b]b" "" # crash test

title "Stack"
run "[]K=\n[+]K=1\nK" 1
run "[]K=\n[+]K=1\n[+]K=2\n[]K" "1\n2"
run "[]K=1,2,3\n[-]K\n[?]K" "3\n2"
run "[]K=1,2,3\n[-]K\n[]K" "3\n1\n2" # XXX
run "[]K=1,2,3\n[-]K\n[+]K=4\n[]K" "3\n1\n2\n4"

title "Negative"
run "a=-2\na" -2
run "a=-3\n+a" 1
run "a=-2\n+a=1\na" 1
run "a=-2\n+a=4\na" 4
run "a=0\n-a" 0
run "a=0\n-a=4\na" 0
run "a=0\n+a=-4\na" 0
run "a=0\n-a=4\na" 0

title "JSON"
run 'foo=[1,2,3]\nfoo:[1]' 2
run 'foo=[1,2,3]\n+foo:[1]\nfoo:[1]' "3\n3"
run 'foo=[1,2,3]\nfoo:[1]=999\nfoo' '[1,999,3]'
run 'foo={"bar":"V"}\nfoo:bar' V
run 'foo={"bar":123}\nfoo:bar' 123
run 'foo={"bar":123}\nfoo:bar=69\nfoo:bar' 69
run 'foo={"bar":[1,2]}\nfoo:bar[0]' 1

title "Slurp"
printf "K=V\nK\n" > .t
run "..t" V
run "..f" '' 2>/dev/null
run "..t\n..t" "V\nV"
rm -f .t .f

title "Results"

TOTAL=$((${SUCCESS}+${FAILED}))
#RATIO=`echo "100 ${FAILED} * ${TOTAL} / n" | dc`
RATIO=$(((100*${FAILED})/${TOTAL}))
echo "  TOTAL       ${TOTAL}" >/dev/stderr
echo "  SUCCESS     ${SUCCESS}" >/dev/stderr
echo "  FAILED      ${FAILED}" >/dev/stderr
echo "  BROKENNESS  ${RATIO}%" > /dev/stderr
