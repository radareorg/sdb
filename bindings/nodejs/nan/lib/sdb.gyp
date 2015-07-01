{
  "targets": [
    {
      "target_name": "libsdb",
      "type": "static_library",
      "include_dirs": [
        "sdb/src"
      ],
      "sources": [
        "sdb/src/sdb.c",
        "sdb/src/fmt.c",
	"sdb/src/array.c",
	"sdb/src/base64.c",
	"sdb/src/buffer.c",
	"sdb/src/cdb.c",
	"sdb/src/cdb_make.c",
	"sdb/src/disk.c",
	"sdb/src/ht.c",
	"sdb/src/journal.c",
	"sdb/src/json.c",
	"sdb/src/lock.c",
	"sdb/src/ls.c",
	"sdb/src/match.c",
	"sdb/src/ns.c",
	"sdb/src/num.c",
	"sdb/src/query.c",
	"sdb/src/sdb_version.h",
	"sdb/src/util.c",
      ]
    }
  ]
}
