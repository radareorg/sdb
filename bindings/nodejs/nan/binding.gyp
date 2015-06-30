{
  "targets": [
    {
      "target_name": "sdb",
      "sources": [ "sdb.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "lib/sdb/src"
      ],
      "dependencies": [
        "lib/binding.gyp:libsdb"
      ]
    }
  ]
}
