{
  "targets": [
    {
      "target_name": "sdb",
      "sources": [ "sdb.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "../../../src"
      ],
      "libraries": [
        "../../../../src/libsdb.a"
      ]
    }
  ]
}
