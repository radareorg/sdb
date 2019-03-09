tests = [
  'array',
  'new',
  'reset',
  'set',
  'stack',
  'sync'
]

foreach test : tests
  name = 'bench-' + test
  exe = executable(name, name + '.c',
    include_directories: sdb_inc,
    link_with: libsdb.get_static_lib()
  )
  benchmark(name, exe)
endforeach
