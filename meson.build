project('sdb', 'c')
version = '1.0.0'

files = [
	'src/array.c',
	'src/base64.c',
	'src/buffer.c',
	'src/cdb.c',
	'src/cdb_make.c',
	'src/disk.c',
	'src/fmt.c',
	'src/ht.c',
	'src/journal.c',
	'src/json.c',
	# 'src/json/api.c',
#	'src/json/indent.c',
	'src/json/js0n.c',
	'src/json/path.c',
	# 'src/json/rangstr.c',
	'src/lock.c',
	'src/ls.c',
	'src/match.c',
	'src/ns.c',
	'src/num.c',
	'src/query.c',
	'src/sdb.c',
	'src/sdbht.c',
	'src/util.c',
]

libsdb = static_library('sdb', files,
  include_directories: include_directories(['src'])
)

executable('sdb', 'src/main.c',
  include_directories: include_directories(['src']),
  link_with: [libsdb],
  install: true
)
