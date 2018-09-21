prefix=@@PREFIX@@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: sdb
Description: Simple DataBase
Version: @@VERSION@@
Requires:
Libs: -L${libdir} -lsdb
Cflags: -I${includedir}/sdb -I${includedir}
