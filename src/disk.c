/* sdb - MIT - Copyright 2013-2024 - pancake */

#include <fcntl.h>
#include "sdb_private.h"

SDB_API bool sdb_disk_create(Sdb* s) {
	int nlen;
	char *str;
	const char *dir;
	if (!s || s->fdump >= 0) {
		return false; // cannot re-create
	}
	if (!s->dir && s->name) {
		s->dir = sdb_strdup (s->name);
	}
	dir = s->dir ? s->dir : "./";
	R_FREE (s->ndump);
	nlen = strlen (dir);
	str = (char *)sdb_gh_malloc (nlen + 5);
	if (!str) {
		return false;
	}
	memcpy (str, dir, nlen + 1);
	sdb_file_mkdirp (str);
	memcpy (str + nlen, ".tmp", 5);
	s->fdump = sdb_file_open (str, O_RDWR | O_CREAT | O_TRUNC, SDB_MODE);
	if (s->fdump == -1) {
		// eprintf ("sdb: Cannot open '%s' for writing.\n", str);
		sdb_gh_free (str);
		return false;
	}
	s->ndump = str;
	if (!cdb_make_start (&s->m, s->fdump)) {
		sdb_disk_abort (s);
		return false;
	}
	return true;
}

SDB_API bool sdb_disk_insert(Sdb* s, const char *key, const char *val) {
	if (!s || !key || !val) {
		return false;
	}
	//if (!*val) return 0; //undefine variable if no value
	return cdb_make_add (&s->m, key, strlen (key), val, strlen (val));
}

static void close_current_database(Sdb *s) {
	cdb_fini (&s->db);
	s->db.fd = -1;
	if (s->fd != -1) {
		close (s->fd);
		s->fd = -1;
	}
}

SDB_IPI void sdb_disk_abort(Sdb *s) {
	if (!s) {
		return;
	}
	cdb_make_cancel (&s->m);
	if (s->fdump >= 0) {
		close (s->fdump);
	}
	s->fdump = -1;
	if (s->ndump) {
		sdb_file_unlink (s->ndump);
		R_FREE (s->ndump);
	}
}

SDB_API bool sdb_disk_finish(Sdb* s) {
	if (!s) {
		return false;
	}
	if (s->fdump < 0 || !s->ndump || !s->dir) {
		sdb_disk_abort (s);
		return false;
	}
	bool ret = cdb_make_finish (&s->m);
#if USE_MMAN
	if (ret && fsync (s->fdump) == -1) {
		ret = false;
	}
#endif
	if (close (s->fdump) == -1) {
		ret = false;
	}
	s->fdump = -1;
	if (!ret) {
		sdb_disk_abort (s);
		return false;
	}
	// Windows cannot replace a file this process still has open
	close_current_database (s);
	if (!sdb_file_replace (s->ndump, s->dir)) {
		sdb_disk_abort (s);
		(void)sdb_open (s, s->dir);
		return false;
	}
	sdb_gh_free (s->ndump);
	s->ndump = NULL;
	return sdb_open (s, s->dir) >= 0;
}

SDB_API bool sdb_disk_unlink(Sdb *s) {
	if (!s) {
		return false;
	}
	// the file must be closed and unmapped or the unlink fails on windows
	close_current_database (s);
	return s->dir && *s->dir && sdb_file_unlink (s->dir);
}
