#ifndef SDB_PRIVATE_H
#define SDB_PRIVATE_H

#include <stdarg.h>
#include "sdb/sdb.h"

SDB_IPI bool sdb_vfmtkey(char key[SDB_MAX_KEY], const char *fmt, va_list ap);
SDB_IPI char *sdb_vstrdupf(const char *fmt, va_list ap);
SDB_IPI const char *sdb_const_vgetf(Sdb *s, ut32 *cas, const char *fmt, va_list ap);
SDB_IPI bool sdb_vsetf(Sdb *s, const char *val, ut32 cas, const char *fmt, va_list ap);
SDB_IPI void sdb_disk_abort(Sdb *s);

#endif
