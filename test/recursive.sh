#!/bin/sh
. ./sdb-test.sh
set -eu
SDB="$(cd "$(dirname "$SDB")" && pwd)/$(basename "$SDB")"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir "$tmp/in" "$tmp/out"
cat > "$tmp/in/a.sdb.txt" <<'EOF'
foo=bar
comma,key=comma,value
quote"key=quote"value
back\\key=back\\value
return\rkey=return\rvalue
newline\nkey=newline\nvalue
tab\tkey=tab\tvalue
EOF
cat > "$tmp/check.c" <<'EOF'
#include <stdio.h>
#include <string.h>

const char *gperf_a_get(const char *key);

static int check(const char *key, const char *expected) {
	const char *value = gperf_a_get (key);
	if (!value || strcmp (value, expected)) {
		fprintf (stderr, "FAIL: generated lookup for %s\n", key);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv) {
	return check ("foo", "bar")
		|| check (argc > 1? argv[1]: "comma,key", "comma,value")
		|| check ("quote\"key", "quote\"value")
		|| check ("back\\key", "back\\value")
		|| check ("return\rkey", "return\rvalue")
		|| check ("newline\nkey", "newline\nvalue")
		|| check ("tab\tkey", "tab\tvalue");
}
EOF

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
	${CC:-cc} "$dir/a.c" "$tmp/check.c" -o "$dir/check"
	"$dir/check"
	"$SDB" -C -t -o "$tmp/cli.c" "$dir/a.sdb"
	${CC:-cc} "$tmp/cli.c" "$tmp/check.c" -o "$tmp/check"
	"$tmp/check" comma_key
done
