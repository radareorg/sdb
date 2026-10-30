#ifndef SDB_PRIVATE_H
#define SDB_PRIVATE_H

#include <stdarg.h>
#include "sdb/sdb.h"

SDB_IPI bool sdb_vfmtkey(char key[SDB_MAX_KEY], const char *fmt, va_list ap);
SDB_IPI char *sdb_vstrdupf(const char *fmt, va_list ap);
SDB_IPI const char *sdb_const_vgetf(Sdb *s, ut32 *cas, const char *fmt, va_list ap);
SDB_IPI bool sdb_vsetf(Sdb *s, const char *val, ut32 cas, const char *fmt, va_list ap);
SDB_IPI void sdb_disk_abort(Sdb *s);

/* file.c */
#if __SDB_WINDOWS__
SDB_IPI wchar_t *sdb_file_wide(const char *path);
#endif
SDB_IPI int sdb_file_open(const char *path, int flags, int mode);
SDB_IPI bool sdb_file_unlink(const char *path);
SDB_IPI bool sdb_file_mkdir(const char *path);
SDB_IPI bool sdb_file_mkdirp(char *path);
SDB_IPI bool sdb_file_replace(const char *src, const char *dst);

/* lock.c: an Sdb holds at most one lock, on "<dir>.lock", from
 * sdb_lock_db until sdb_unlock_db, which sdb_file and sdb_free call.
 * mode 1 fails if the lock is taken, mode 2 waits for it */
SDB_IPI bool sdb_lock_db(Sdb *s, int mode);
SDB_IPI void sdb_unlock_db(Sdb *s);

#endif
