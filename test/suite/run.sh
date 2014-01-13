#!/bin/sh

TOTAL=0
SUCCESS=0
FAILED=0

run() {
	K="$1"
	A=`echo "$1" | sdb -`
	B="`echo \"$2\"`"
	GREEN="\x1b[32m"
	RED="\x1b[31m"
	RESET="\x1b[0m"
	if [ "$A" = "$B" ]; then
		echo "   ${GREEN}OK${RESET}  - "`printf "$B"`" = "`printf "$K"`
		SUCCESS=$((${SUCCESS}+1))
	else
		echo " ${RED}ERROR${RESET} - "`printf "$B"`" = "`printf "$K"`"  =>  "`printf "$A"`
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

title "Numbers"
run "K=0\n+K" 1
run "K=1\n-K" 0
run "K=0\n-K" 0       # ?? should we allow negative values?
run "K=0\n+K=1\nK" 1
run "K=0\n+K=2\nK" 2
run "K=0\n-K=10\nK" 0
run "K=-1\n+K" 0

title "Arrays"
run "()K=1,2,3\n(?)K" 3

title "JSON"
run 'foo=[1,2,3]\nfoo?[1]' 2
run 'foo=[1,2,3]\nfoo?[1]=999\nfoo' '[1,999,3]'
run 'foo={bar:"V"}\nfoo?bar' V

title "Slurp"
printf "K=V\nK\n" > .t

run "<.t" V
run "<.f" ''
run "<.t\n<.t" "V\nV"

rm -f .t .f



title "Results"

TOTAL=$((${SUCCESS}+${FAILED}))
RATIO=`echo "100 ${FAILED} * ${TOTAL} / n" | dc`
echo "  TOTAL       ${TOTAL}"
echo "  SUCCESS     ${SUCCESS}"
echo "  FAILED      ${FAILED}"
echo "  BROKENNESS  ${RATIO}%"
