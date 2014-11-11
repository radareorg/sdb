prefix=@@PREFIX@@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/sdb

Name: mcsdb
Description: Memcache on Simple DataBase
Version: @@VERSION@@
Requires:
Libs: -L${libdir} -lmcsdb
Cflags: -I${includedir}
