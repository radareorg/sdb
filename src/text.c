/* sdb - MIT - Copyright 2020 - thestr4ng3r */

#include "sdb.h"

/**
 * ********************
 * Plaintext SDB Format
 * ********************
 *
 * Files are UTF-8 and use '\n' line endings. Always.
 * 
 * Lines starting with '/' denote the path of the namespace for the following data:
 *
 *   /some/namespace
 * 
 * The default path is root, just a slash also means root.
 * These paths are always absolute from the root. Characters that must be escaped in a path are: '/', '\\', '\n':
 *
 *   /s\/ome/name\nspa\\ce
 *
 * SDB entries are written each as a single k=v line:
 *
 *   somekey=somevalue
 *
 * To distinguish these from path lines, if there is a leading '/' in the key, it must be escaped
 * (slashes later in the line don't have to be escaped):
 *
 *   \/slashedkey=somevalue
 *
 * Other than that, at any postion, '\\' and '\n' must be escaped:
 *   
 *   some\\key=some\nvalue
 *
 * In the key, '=' must also be escaped (not necessary in the value):
 *
 *   some\=key=some=value
 * 
 * --------
 * Example:
 *
 *   /
 *   key=intheroot
 *   \/slashedkey=somevalue
 *   some\\key=some\nvalue
 *   some\=key=some=value
 *
 *   /subns
 *   some=stuff in the sub-namespace
 *
 *   /subns/deeper
 *   this=is in /subns/deeper
 *
*/

static int cmp_ns(const void *a, const void *b) {
	const SdbNs *nsa = a;
	const SdbNs *cia = b;
	return strcmp (nsa->name, cia->name);
}

static bool text_fsave(Sdb *s, FILE *f, bool sort, SdbList *path) {
// macro to flush a block of text that doesn't have to be escaped
// n = position we are currently looking at
// p = position until we have already written everything
#define FLUSH do { if (p != n) { fwrite (p, 1, n - p, f); p = n; } } while (0)
	// path
	fwrite ("/", 1, 1, f); // always print a /, even if path is empty
	SdbListIter *it;
	const char *path_token;
	bool first = true;
	ls_foreach (path, it, path_token) {
		if (first) {
			first = false;
		} else {
			fwrite ("/", 1, 1, f);
		}
		// write and escape the path
		const char *p = path_token;
		const char *n = p;
		while (*n) {
			switch (*n) {
			case '\\':
				FLUSH;
				fwrite ("\\\\", 1, 1, f);
				break;
			case '/':
				FLUSH;
				fwrite ("\\/", 1, 1, f);
				break;
			case '\n':
				FLUSH;
				fwrite ("\\n", 1, 1, f);
				break;
			}
			n++;
		}
		FLUSH;
	}
	fwrite ("\n", 1, 1, f);

	// k=v entries
	SdbList *l = sdb_foreach_list (s, sort);
	SdbKv *kv;
	ls_foreach (l, it, kv) {
		const char *k = sdbkv_key (kv);
		const char *v = sdbkv_value (kv);
		// escape leading '/'
		if (*k == '/') {
			fwrite ("\\", 1, 1, f);
		}

		// write and escape key
		const char *p = k;
		const char *n = p;
		while (*n) {
			switch (*n) {
			case '\\':
				FLUSH;
				fwrite ("\\\\", 1, 1, f);
				break;
			case '=':
				FLUSH;
				fwrite ("\\=", 1, 1, f);
				break;
			case '\n':
				FLUSH;
				fwrite ("\\n", 1, 1, f);
				break;
			}
			n++;
		}
		FLUSH;

		fwrite ("=", 1, 1, f);

		// write and escape value
		p = v;
		n = p;
		while (*n) {
			switch (*n) {
			case '\\':
				FLUSH;
				fwrite ("\\\\", 1, 1, f);
				break;
			case '\n':
				FLUSH;
				fwrite ("\\n", 1, 1, f);
				break;
			}
			n++;
		}
		FLUSH;
	}
	ls_free (l);

	// sub-namespaces
	l = s->ns;
	if (sort) {
		l = ls_clone (l);
		ls_sort (l, cmp_ns);
	}
	SdbNs *ns;
	ls_foreach (l, it, ns) {
		ls_push (path, ns->name);
		text_fsave(ns->sdb, f, sort, path);
		ls_pop (path);
	}
	if (l != s->ns) {
		ls_free (l);
	}

#undef FLUSH
	return false;
}

SDB_API bool sdb_text_fsave(Sdb *s, FILE *f, bool sort) {
	SdbList *path = ls_new ();
	if (!path) {
		return false;
	}
	bool r = text_fsave (s, f, sort, NULL);
	ls_free (path);
	return r;
}

SDB_API bool sdb_text_save(Sdb *s, const char *file, bool sort) {
	FILE *f = fopen (file, "w");
	if (!f) {
		return false;
	}
	bool r = sdb_text_fsave (s, f, sort);
	fclose (f);
	return r;
}

SDB_API bool sdb_text_fload(Sdb *s, FILE *f) {
	// TODO
	return false;
}

SDB_API bool sdb_text_load(Sdb *s, const char *file) {
	FILE *f = fopen (file, "r");
	if (!f) {
		return false;
	}
	bool r = sdb_text_fload (s, f);
	fclose (f);
	return r;
}

