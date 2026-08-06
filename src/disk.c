/* sdb - MIT - Copyright 2013-2024 - pancake */

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "sdb_private.h"

#if __SDB_WINDOWS__

#if UNICODE

static wchar_t *r_utf8_to_utf16_l(const char *cstring, int len) {
	if (!cstring || !len || len < -1) {
		return NULL;
	}
	wchar_t *rutf16 = NULL;
	int wcsize;

	if ((wcsize = MultiByteToWideChar (CP_UTF8, 0, cstring, len, NULL, 0))) {
		wcsize += 1;
		if ((rutf16 = (wchar_t *) sdb_gh_calloc (wcsize, sizeof (wchar_t)))) {
			MultiByteToWideChar (CP_UTF8, 0, cstring, len, rutf16, wcsize);
			if (len != -1) {
				rutf16[wcsize - 1] = L'\0';
			}
		}
	}
	return rutf16;
}

#define r_sys_conv_utf8_to_utf16(buf) r_utf8_to_utf16_l ((buf), -1)

static bool r_sys_mkdir(const char *path) {
	LPTSTR path_ = r_sys_conv_utf8_to_utf16 (path);
	bool ret = CreateDirectory (path_, NULL);
	sdb_gh_free (path_);
	return ret;
}
#else
#define r_sys_conv_utf8_to_utf16(buf) sdb_strdup (buf)
#define r_sys_mkdir(x) CreateDirectory (x, NULL)
#endif
#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS 183
#endif
#define r_sys_mkdir_failed() (GetLastError () != 183)
#else
#define r_sys_mkdir(x) (mkdir (x,0755)!=-1)
#define r_sys_mkdir_failed() (errno != EEXIST)
#endif

static bool unlink_path(const char *path) {
	if (!path) {
		return false;
	}
#if __SDB_WINDOWS__ && UNICODE
	wchar_t *wpath = r_sys_conv_utf8_to_utf16 (path);
	bool ret = wpath && _wunlink (wpath) != -1;
	sdb_gh_free (wpath);
	return ret;
#else
	return unlink (path) != -1;
#endif
}

static inline bool mkdirp(char *dir) {
	const char slash = DIRSEP;
	char *path = dir;
	char *ptr = path;
	if (*ptr == slash) {
		ptr++;
	}
#if __SDB_WINDOWS__
	char *p = strstr (ptr, ":\\");
	if (p) {
		ptr = p + 2;
	}
#endif
	while ((ptr = strchr (ptr, slash))) {
		*ptr = 0;
		if (!r_sys_mkdir (path) && r_sys_mkdir_failed ()) {
			// eprintf ("cannot make directory r_sys_mkdirp: fail '%s' of '%s'\n", path, dir);
			*ptr = slash;
			return false;
		}
		*ptr = slash;
		ptr++;
	}
	return true;
}

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
	mkdirp (str);
	memcpy (str + nlen, ".tmp", 5);
#if __SDB_WINDOWS__ && UNICODE
	wchar_t *wstr = r_sys_conv_utf8_to_utf16 (str);
	if (wstr) {
		s->fdump = _wopen (wstr, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, SDB_MODE);
		sdb_gh_free (wstr);
	} else {
		s->fdump = -1;
	}
#else
	s->fdump = open (str, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, SDB_MODE);
#endif
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
	cdb_free (&s->db);
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
		unlink_path (s->ndump);
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
#if __SDB_WINDOWS__
	LPTSTR ndump_ = r_sys_conv_utf8_to_utf16 (s->ndump);
	LPTSTR dir_ = r_sys_conv_utf8_to_utf16 (s->dir);
	if (!ndump_ || !dir_) {
		sdb_gh_free (ndump_);
		sdb_gh_free (dir_);
		sdb_disk_abort (s);
		return false;
	}
	// Windows cannot replace a file while the existing database is open.
	close_current_database (s);
	ret = MoveFileEx (ndump_, dir_, MOVEFILE_REPLACE_EXISTING) != 0;
	sdb_gh_free (ndump_);
	sdb_gh_free (dir_);
	if (!ret) {
		sdb_disk_abort (s);
		(void)sdb_open (s, s->dir);
		return false;
	}
#else
	if (rename (s->ndump, s->dir) == -1) {
		sdb_disk_abort (s);
		return false;
	}
	close_current_database (s);
#endif
	sdb_gh_free (s->ndump);
	s->ndump = NULL;
	return sdb_open (s, s->dir) >= 0;
}

SDB_API bool sdb_disk_unlink(Sdb *s) {
	return s->dir && *s->dir && unlink_path (s->dir);
}
