
if get_option('enable_tests')
  tests = [
    'array',
    'diff',
    'hash',
    'ls',
    'sdb'
  ]

  foreach test : tests
    exe = executable('test_@0@'.format(test), 'test_@0@.c'.format(test),
      dependencies: [sdb_dep],
      install: false,
      implicit_include_directories: false
    )
    test(test, exe, workdir: join_paths(meson.current_source_dir(), '..'))
  endforeach
endif
