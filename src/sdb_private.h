#ifndef SDB_PRIVATE_H
#define SDB_PRIVATE_H

#include <stdarg.h>
#include "sdb/sdb.h"

SDB_IPI const char *sdb_const_vgetf(Sdb *s, ut32 *cas, const char *fmt, va_list ap);

#endif
