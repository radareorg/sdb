#!/bin/sh
. ./sdb-test.sh
set -eu

check() {
	expected=$1
	input=$2
	shift 2
	output=$(printf '%s' "$input" | "$SDB" "$@") || {
		echo "FAIL: $* returned a failure status" >&2
		exit 1
	}
	if [ "$output" != "$expected" ]; then
		echo "FAIL: unexpected output from $*" >&2
		exit 1
	fi
}

version=$(sed -n 's/^#define SDB_VERSION "\(.*\)"/sdb \1/p' ../include/sdb/version.h)
check "$version" '' -v
check "$version" '' -v -z
help=$("$SDB" -h)
[ "$(printf '%s\n' "$help" | grep -c '^usage:')" -eq 1 ]
check 'aGVsbG8=' 'hello' -e
check 'hello' 'aGVsbG8=' -d
check '{
  "a": 1
}' '{"a":1}' -j

for flag in -z -g -D -c; do
	if "$SDB" "$flag" >/dev/null 2>&1; then
		echo "FAIL: $flag should fail" >&2
		exit 1
	fi
done
