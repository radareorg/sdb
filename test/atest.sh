a="`echo '
()list=one,two,tri,fur
(0)list
(0)list=foo
(0)list
(+0)list=zero
()list
' | sdb -`"
r="one
foo
zero
foo
two
tri
fur"

#echo "$a"
#echo ===
#echo "$r"

if [ "$a" = "$r" ]; then
	echo "OK"
else
	echo "FAIL"
fi
