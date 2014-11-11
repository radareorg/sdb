prefix=@@PREFIX@@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include/sdb

Name: sdb
Description: Simple DataBase
Version: @@VERSION@@
Requires:
Libs: -L${libdir} -lsdb
Cflags: -I${includedir}
