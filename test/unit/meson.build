
if get_option('enable_tests')
  # Sync tests use internal cdb functions that are not exported by the DLL.
  sync_dep = declare_dependency(
    link_with: libsdb_static_link,
    include_directories: sdb_inc
  )

  tests = [
    'array',
    'diff',
    'hash',
    'ls',
    'match',
    'sdb',
    'sync'
  ]

  foreach test : tests
    exe = executable('test_@0@'.format(test), 'test_@0@.c'.format(test),
      dependencies: test == 'sync' ? sync_dep : sdb_dep,
      install: false,
      implicit_include_directories: false
    )
    test(test, exe, workdir: join_paths(meson.current_source_dir(), '..'))
  endforeach
endif
