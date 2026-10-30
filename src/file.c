/* sdb - MIT - Copyright 2013-2026 - pancake */

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "sdb_private.h"

#if __SDB_WINDOWS__
// FILE_RENAME_INFO with the Flags member missing from older headers
typedef struct {
	DWORD Flags;
	HANDLE RootDirectory;
	DWORD FileNameLength;
	WCHAR FileName[1];
} SdbRenameInfo;

#define SDB_FILE_RENAME_INFO_EX 22 // FileRenameInfoEx
#define SDB_RENAME_REPLACE_IF_EXISTS 1
#define SDB_RENAME_POSIX_SEMANTICS 2

// resolved at runtime, older headers (_WIN32_WINNT < 0x0600) lack the declarations
typedef BOOL (WINAPI *SdbSetFileInfo)(HANDLE, int, LPVOID, DWORD);

// paths are UTF-8 in UNICODE builds and in the ANSI codepage otherwise
SDB_IPI wchar_t *sdb_file_wide(const char *path) {
#if UNICODE
	const UINT cp = CP_UTF8;
#else
	const UINT cp = CP_ACP;
#endif
	int n = path? MultiByteToWideChar (cp, 0, path, -1, NULL, 0): 0;
	wchar_t *w = (n > 0)? (wchar_t *)sdb_gh_calloc (n, sizeof (wchar_t)): NULL;
	if (w && !MultiByteToWideChar (cp, 0, path, -1, w, n)) {
		R_FREE (w);
	}
	return w;
}

// Replace dst even while other processes have it open with FILE_SHARE_DELETE,
// their handles keep reading the old file (Windows 10 1809+ on NTFS)
static bool posix_rename(const wchar_t *src, const wchar_t *dst) {
	bool ret = false;
	HMODULE k32 = GetModuleHandleW (L"kernel32.dll");
	SdbSetFileInfo set_info = k32? (SdbSetFileInfo)(void *)GetProcAddress (k32, "SetFileInformationByHandle"): NULL;
	if (!set_info) {
		return false;
	}
	DWORD len = GetFullPathNameW (dst, 0, NULL, NULL);
	size_t size = sizeof (SdbRenameInfo) + len * sizeof (WCHAR);
	SdbRenameInfo *ri = len? (SdbRenameInfo *)sdb_gh_calloc (1, size): NULL;
	if (!ri) {
		return false;
	}
	ri->Flags = SDB_RENAME_REPLACE_IF_EXISTS | SDB_RENAME_POSIX_SEMANTICS;
	ri->FileNameLength = GetFullPathNameW (dst, len, ri->FileName, NULL) * sizeof (WCHAR);
	HANDLE h = CreateFileW (src, DELETE | SYNCHRONIZE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) {
		ret = ri->FileNameLength && set_info (h, SDB_FILE_RENAME_INFO_EX, ri, (DWORD)size);
		CloseHandle (h);
	}
	sdb_gh_free (ri);
	return ret;
}
#endif

// supports O_RDONLY, O_RDWR, O_WRONLY, O_CREAT, O_TRUNC and O_APPEND
SDB_IPI int sdb_file_open(const char *path, int flags, int mode) {
#if __SDB_WINDOWS__
	// share delete access so other processes can replace the file while it is open
	wchar_t *wpath = sdb_file_wide (path);
	if (!wpath) {
		return -1;
	}
	const bool rw = flags & (O_WRONLY | O_RDWR);
	DWORD disposition = OPEN_EXISTING;
	if (flags & O_CREAT) {
		disposition = (flags & O_TRUNC)? CREATE_ALWAYS: OPEN_ALWAYS;
	}
	HANDLE h = CreateFileW (wpath, rw? GENERIC_READ | GENERIC_WRITE: GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
	sdb_gh_free (wpath);
	if (h == INVALID_HANDLE_VALUE) {
		return -1;
	}
	int fd = _open_osfhandle ((intptr_t)h, flags & (O_WRONLY | O_RDWR | O_APPEND));
	if (fd == -1) {
		CloseHandle (h);
	}
	return fd;
#else
	return open (path, flags | O_BINARY, mode);
#endif
}

SDB_IPI bool sdb_file_unlink(const char *path) {
#if __SDB_WINDOWS__
	wchar_t *wpath = sdb_file_wide (path);
	bool ret = wpath && _wunlink (wpath) != -1;
	sdb_gh_free (wpath);
	return ret;
#else
	return path && unlink (path) != -1;
#endif
}

// succeeds if the directory already exists
SDB_IPI bool sdb_file_mkdir(const char *path) {
#if __SDB_WINDOWS__
	wchar_t *wpath = sdb_file_wide (path);
	bool ret = wpath && (CreateDirectoryW (wpath, NULL)
		|| GetLastError () == ERROR_ALREADY_EXISTS);
	sdb_gh_free (wpath);
	return ret;
#else
	return path && (mkdir (path, 0755) != -1 || errno == EEXIST);
#endif
}

// create the parent directories of path
SDB_IPI bool sdb_file_mkdirp(char *path) {
	const char slash = DIRSEP;
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
		bool ok = sdb_file_mkdir (path);
		*ptr++ = slash;
		if (!ok) {
			return false;
		}
	}
	return true;
}

// atomically replace dst with src
SDB_IPI bool sdb_file_replace(const char *src, const char *dst) {
#if __SDB_WINDOWS__
	wchar_t *wsrc = sdb_file_wide (src);
	wchar_t *wdst = sdb_file_wide (dst);
	bool ret = wsrc && wdst && (posix_rename (wsrc, wdst)
		|| MoveFileExW (wsrc, wdst, MOVEFILE_REPLACE_EXISTING));
	sdb_gh_free (wsrc);
	sdb_gh_free (wdst);
	return ret;
#else
	return rename (src, dst) != -1;
#endif
}
