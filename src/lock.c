/* sdb - MIT - Copyright 2012-2022 - pancake */

#include <fcntl.h>
#include "sdb/sdb.h"

SDB_API bool sdb_lock_file(const char *f, char *buf, size_t buf_size) {
	if (!f || !*f || !buf || !buf_size) {
		return false;
	}
	snprintf (buf, buf_size, "%s.lock", f);
	return true;
}

#if __SDB_WINDOWS__
// The lock is an exclusive handle that deletes the file on close,
// so the OS releases it even if the process crashes or gets killed.
typedef struct sdb_lock_t {
	char *path;
	HANDLE h;
	struct sdb_lock_t *next;
} SdbLock;

static SdbLock *locks = NULL;

SDB_API bool sdb_lock(const char *s) {
	if (!s) {
		return false;
	}
	HANDLE h = CreateFileA (s, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	SdbLock *l = (SdbLock *)sdb_gh_calloc (1, sizeof (SdbLock));
	if (!l || !(l->path = sdb_strdup (s))) {
		sdb_gh_free (l);
		CloseHandle (h);
		return false;
	}
	l->h = h;
	l->next = locks;
	locks = l;
	return true;
}

SDB_API void sdb_unlock(const char *s) {
	SdbLock **pl;
	for (pl = &locks; s && *pl; pl = &(*pl)->next) {
		SdbLock *l = *pl;
		if (!strcmp (l->path, s)) {
			*pl = l->next;
			CloseHandle (l->h);
			sdb_gh_free (l->path);
			sdb_gh_free (l);
			return;
		}
	}
}
#else
SDB_API bool sdb_lock(const char *s) {
	char pidstr[64];
	if (!s) {
		return false;
	}
	int fd = open (s, O_CREAT | O_TRUNC | O_WRONLY | O_EXCL, SDB_MODE);
	if (fd == -1) {
		return false;
	}
#if __wasi__ || EMSCRIPTEN || __MINGW32__
	char *pid = sdb_itoa (rand (), 10, pidstr, sizeof (pidstr));
#else
	char *pid = sdb_itoa (getpid (), 10, pidstr, sizeof (pidstr));
#endif
	if (pid) {
		if ((write (fd, pid, strlen (pid)) < 0)
			|| (write (fd, "\n", 1) < 0)) {
			close (fd);
			unlink (s);
			return false;
		}
	}
	close (fd);
	return true;
}

SDB_API void sdb_unlock(const char *s) {
	//flock (fd, LOCK_UN);
	unlink (s);
}
#endif

SDB_API bool sdb_lock_wait(const char *s) {
	// TODO use flock() here
	// wait forever here?
 	while (!sdb_lock (s)) {
		// TODO: if waiting too much return 0
#if __SDB_WINDOWS__
	 	Sleep (500); // hack
#else
		// TODO use lockf() here .. flock is not much useful (fd, LOCK_EX);
	 	sleep (1); // hack
#endif
 	}
	return true;
}

#if TEST
main () {
	int r;
	r = sdb_lock (".lock");
	printf ("%d\n", r);
	r = sdb_lock (".lock");
	printf ("%d\n", r);
	sdb_unlock (".lock");
	r = sdb_lock (".lock");
	printf ("%d\n", r);
}
#endif
