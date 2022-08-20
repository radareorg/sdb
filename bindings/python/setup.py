#!/usr/bin/env python3

from distutils.core import setup, Extension

setup(
	name = "sdb",
	version = "1.8.8",
	ext_modules = [Extension("sdb",
		 ["pysdb.c",
"../../src/cdb_make.c",
"../../src/dict.c",
"../../src/match.c",
"../../src/json.c",
"../../src/diff.c",
"../../src/buffer.c",
"../../src/ls.c",
"../../src/util.c",
"../../src/array.c",
"../../src/cdb.c",
"../../src/journal.c",
"../../src/ht_uu.c",
"../../src/set.c",
"../../src/sdb.c",
"../../src/ht_pu.c",
"../../src/sdbht.c",
#"../../src/json/api.c",
#"../../src/json/rangstr.c",
#"../../src/json/main.c",
#"../../src/json/js0n.c",
#"../../src/json/path.c",
#"../../src/json/test.c",
#"../../src/json/indent.c",
"../../src/ht_up.c",
"../../src/main.c",
"../../src/query.c",
"../../src/base64.c",
"../../src/text.c",
"../../src/ht_pp.c",
"../../src/num.c",
"../../src/lock.c",
"../../src/disk.c",
"../../src/fmt.c",
"../../src/ns.c",

],
		 include_dirs = ["../../src"]
)]
		 #extra_compile_args = ["-L../../src", "../../src/libsdb.a"])]
     );
