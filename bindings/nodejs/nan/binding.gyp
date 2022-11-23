{
  "targets": [
    {
      "type": "loadable_module",
      "target_name": "sdb",
      "sources": [ "sdb.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
	"./lib/sdb/include"
      ],
      "dependencies": [
        "./lib/sdb.gyp:libsdb"
      ]
    }
  ]
}
