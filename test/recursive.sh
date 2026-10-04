#!/bin/sh
. ./sdb-test.sh
set -eu
SDB="$(cd "$(dirname "$SDB")" && pwd)/$(basename "$SDB")"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir "$tmp/in" "$tmp/out"
printf 'foo=bar\n' > "$tmp/in/a.sdb.txt"

for mode in local output; do
	if [ "$mode" = local ]; then
		"$SDB" -r -r "$tmp/in"
		dir="$tmp/in"
	else
		SDB_OUTPUT_DIR="$tmp/out" "$SDB" -r -r "$tmp/in"
		dir="$tmp/out"
	fi
	test -s "$dir/a.c"
	test ! -e "$dir/a.sdc"
	test -s "$dir/a.sdb"
	${CC:-cc} -Dsdb_strdup=strdup -DMAIN=1 "$dir/a.c" -o "$dir/check"
	test "$("$dir/check")" = bar
	test "$("$SDB" "$dir/a.sdb" bar)" = foo
done
