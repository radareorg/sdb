#!/usr/bin/env python3

from distutils.core import setup, Extension

setup(
	name = "r2sdb",
	version = "1.8.1",
	description = "string database from radareorg",
	author = "pancake",
	author_email = "pancake@nopcode.org",
	url="https://www.radare.org",
	license = "MIT",
	include_package_data = True,
	package_data = {
		'sdbsrc': ['sdbsrc/*.h'],
		'static': ['*.md'],
	},
	packages=["r2sdb"],
	ext_modules = [
		Extension("nsdb", [
			"nsdb/lib.c",
			"sdbsrc/cdb_make.c",
			"sdbsrc/dict.c",
			"sdbsrc/match.c",
			"sdbsrc/json.c",
			"sdbsrc/diff.c",
			"sdbsrc/buffer.c",
			"sdbsrc/ls.c",
			"sdbsrc/util.c",
			"sdbsrc/array.c",
			"sdbsrc/cdb.c",
			"sdbsrc/journal.c",
			"sdbsrc/ht_uu.c",
			"sdbsrc/set.c",
			"sdbsrc/sdb.c",
			"sdbsrc/ht_pu.c",
			"sdbsrc/sdbht.c",
			#"sdbsrc/json/api.c",
			#"sdbsrc/json/rangstr.c",
			#"sdbsrc/json/main.c",
			#"sdbsrc/json/js0n.c",
			#"sdbsrc/json/path.c",
			#"sdbsrc/json/test.c",
			#"sdbsrc/json/indent.c",
			"sdbsrc/ht_up.c",
			"sdbsrc/main.c",
			"sdbsrc/query.c",
			"sdbsrc/base64.c",
			"sdbsrc/text.c",
			"sdbsrc/ht_pp.c",
			"sdbsrc/num.c",
			"sdbsrc/lock.c",
			"sdbsrc/disk.c",
			"sdbsrc/fmt.c",
			"sdbsrc/ns.c",
			],
			include_dirs = ["sdbsrc"]
		)
	]
	#extra_compile_args = ["-L../../src", "../../src/libsdb.a"])]
)
