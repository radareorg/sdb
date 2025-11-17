project('sdb', 'c', meson_version : '>=0.60.0', version : '2.2.2')

pkgconfig_mod = import('pkgconfig')

sdb_version = meson.project_version()

is_windows = host_machine.system() == 'windows' ? true: false

if is_windows
  libsdb_name = 'libsdb'
  sdb_libversion = ''
else
  sdb_libversion = sdb_version
  libsdb_name = 'sdb'
endif

libsdb_sources = [
  'src/array.c',
  'src/set.c',
  'src/cgen.c',
  'src/strbuf.c',
  'src/tool.c',
  'src/base64.c',
  'src/cdb.c',
  'src/cdb_make.c',
  'src/dict.c',
  'src/diff.c',
  'src/disk.c',
  'src/fmt.c',
  'src/heap.c',
  'src/main.c',
  'src/ht_uu.c',
  'src/ht_up.c',
  'src/ht_pp.c',
  'src/ht_pu.c',
  'src/ht_su.c',
  'src/journal.c',
  'src/json.c',
  'src/lock.c',
  'src/ls.c',
  'src/match.c',
  'src/ns.c',
  'src/num.c',
  'src/query.c',
  'src/sdb.c',
  'src/ht.c',
  'src/util.c',
  'src/text.c'
]

sdb_inc = include_directories(['include'])
rpath_lib = ''
rpath_exe = ''
if get_option('local') and get_option('default_library') == 'shared'
  rpath_lib = '$ORIGIN'
  rpath_exe = '$ORIGIN/../' + get_option('libdir')
endif

libsdb = both_libraries(libsdb_name, libsdb_sources,
  include_directories: sdb_inc,
  implicit_include_directories: false,
  soversion: sdb_libversion,
  install: not meson.is_subproject(),
  install_rpath: rpath_lib
)

libsdb_static = static_library('libsdb_static', libsdb_sources,
  include_directories: sdb_inc,
  implicit_include_directories: false,
  install: not meson.is_subproject(),
  install_rpath: rpath_lib
)

#if is_windows
#  link_with = libsdb.get_static_lib()
#else
#  link_with = libsdb.get_shared_lib()
#endif
default_library_type = get_option('default_library')

if default_library_type == 'shared'
  link_with = libsdb.get_shared_lib()
else
  link_with = libsdb.get_static_lib()
endif

sdb_dep = declare_dependency(
  link_with: [link_with],
  include_directories: sdb_inc
)

sdb_exe = executable('sdb', join_paths('src','entry.c'),
  include_directories: sdb_inc,
  link_with: [link_with],
  install: not meson.is_subproject(),
  install_rpath: rpath_exe,
  implicit_include_directories: false
)

include_files = [
  'include/sdb/version.h',
  'include/sdb/buffer.h',
  'include/sdb/cdb.h',
  'include/sdb/cdb_make.h',
  'include/sdb/config.h',
  'include/sdb/dict.h',
  'include/sdb/heap.h',
  'include/sdb/rangstr.h',
  'include/sdb/ht_inc.h',
  'include/sdb/ht_pp.h',
  'include/sdb/ht_up.h',
  'include/sdb/ht_uu.h',
  'include/sdb/ht_pu.h',
  'include/sdb/ht_su.h',
  'include/sdb/ls.h',
  'include/sdb/sdb.h',
  'include/sdb/ht.h',
  'include/sdb/set.h',
  'include/sdb/types.h',
  'include/sdb/asserts.h',
  'include/sdb/cwisstable.h',
  'include/sdb/gcc_stdatomic.h',
  'include/sdb/msvc_stdatomic.h',
]
if meson.is_subproject()
  install_man([join_paths('src','sdb.1')])
  install_headers(include_files, subdir: 'libr/sdb')
else
  install_headers(include_files, subdir: 'sdb')

  make_exe = find_program('make', required: false)
  if make_exe.found()
    test('run tests', make_exe,
      args: 'test',
      env: ['BASEDIR=' + meson.current_build_dir()],
      workdir: join_paths(meson.current_build_dir(), '..'),
      depends: [sdb_exe, libsdb]
    )
  endif

  subdir(join_paths('test','bench'))
  subdir(join_paths('test','unit'))
endif

if meson.is_cross_build()
  sdb_native_exe = executable('sdb_native', join_paths('src','entry.c'),
    include_directories: sdb_inc,
    link_with: [link_with],
    install: false,
    implicit_include_directories: false
  )
else
  sdb_native_exe = sdb_exe
endif

pkgconfig_mod.generate(
  name: 'sdb',
  filebase: 'sdb',
  libraries: [libsdb.get_shared_lib()],
  description: 'Simple DataBase',
  subdirs: ['sdb'],
  version: sdb_version,
  url: 'https://github.com/radareorg/sdb'
)
