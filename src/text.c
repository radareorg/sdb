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

// n = position we are currently looking at
// p = position until we have already written everything
// macro to flush a block of text that doesn't have to be escaped
#define FLUSH do { if (p != n) { fwrite (p, 1, n - p, f); p = n; } } while (0)
// macro to flush and skip a char
#define SKIP do { FLUSH; p++; } while (0)

static void write_path(FILE *f, SdbList *path) {
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
				SKIP;
				fwrite ("\\\\", 1, 2, f);
				break;
			case '/':
				SKIP;
				fwrite ("\\/", 1, 2, f);
				break;
			case '\n':
				SKIP;
				fwrite ("\\n", 1, 2, f);
				break;
			}
			n++;
		}
		FLUSH;
	}
}

static void write_key(FILE *f, const char *k) {
	const char *p = k;
	const char *n = p;
	while (*n) {
		switch (*n) {
		case '\\':
			SKIP;
			fwrite ("\\\\", 1, 2, f);
			break;
		case '=':
			SKIP;
			fwrite ("\\=", 1, 2, f);
			break;
		case '\n':
			SKIP;
			fwrite ("\\n", 1, 2, f);
			break;
		}
		n++;
	}
	FLUSH;
}

static void write_value(FILE *f, const char *v) {
	// write and escape value
	const char *p = v;
	const char *n = p;
	while (*n) {
		switch (*n) {
		case '\\':
			SKIP;
			fwrite ("\\\\", 1, 2, f);
			break;
		case '\n':
			SKIP;
			fwrite ("\\n", 1, 2, f);
			break;
		}
		n++;
	}
	FLUSH;
}

static bool text_fsave(Sdb *s, FILE *f, bool sort, SdbList *path) {
	//path
	write_path (f, path);
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

		write_key (f, k);
		fwrite ("=", 1, 1, f);
		write_value (f, v);

		fwrite ("\n", 1, 1, f);
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
		fwrite ("\n", 1, 1, f);
		ls_push (path, ns->name);
		text_fsave (ns->sdb, f, sort, path);
		ls_pop (path);
	}
	if (l != s->ns) {
		ls_free (l);
	}

	return true;
}
#undef FLUSH
#undef SKIP

SDB_API bool sdb_text_fsave(Sdb *s, FILE *f, bool sort) {
	SdbList *path = ls_new ();
	if (!path) {
		return false;
	}
	bool r = text_fsave (s, f, sort, path);
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
	size_t bufsz = 16; // TODO: more
	char *buf = malloc (bufsz);
	if (!buf) {
		return false;
	}
	bool ret = true;
	Sdb *cur_db = s;
	// can't use pointers here because buf may be moved on realloc
	size_t pos = 0; // current processing position in the buffer
	size_t token_begin = 0; // beginning of the currently processed token in the buffer
	size_t shift = 0; // amount to shift chars to the left (from unescaping)
	SdbList/*<size_t>*/ *path = ls_new ();
	if (!path) {
		free (buf);
		return false;
	}
	enum { STATE_NEWLINE, STATE_PATH, STATE_KEY, STATE_VALUE } state = STATE_NEWLINE;
	bool unescape = false; // whether the prev char was a backslash, i.e. the current one is escaped
	while (true) {
		// realloc if no space
		if (pos >= bufsz - 1) {
			size_t newsz = bufsz + bufsz;
			if (newsz < bufsz) {
				ret = false;
				break;
			}
			bufsz = newsz;
			char *newbuf = realloc (buf, bufsz);
			if (!newbuf) {
				ret = false;
				break;
			}
			buf = newbuf;
		}
		// read full buffer (leave 1 for null-terminating)
		size_t read = fread (buf + pos, 1, bufsz - 1 - pos, f);
		if (!read && !feof (f)) {
			// error
			ret = false;
			break;
		}
		// process buffer
		size_t buf_filled = pos + read;
		if (!read) {
			goto flush;
		}
		while (pos < buf_filled) {
			if (state == STATE_NEWLINE) {
				// at the start of a line, decide whether it's a path or a k=v
				// by whether there is a leading slash.
				if (buf[pos] == '/') {
					state = STATE_PATH;
					token_begin = 1;
					pos++;
					continue;
				} else {
					state = STATE_KEY;
				}
			}
			if (buf[pos] == '\n') {
				unescape = false;
flush:
				// finish up the line
				buf[pos - shift] = '\0';
				switch (state) {
				case STATE_PATH: {
					ls_push (path, (void *)token_begin);
					token_begin = pos + 1;
					SdbListIter *it;
					void *token_off_tmp;
					cur_db = s;
					ls_foreach (path, it, token_off_tmp) {
						size_t token_off = (size_t)token_off_tmp;
						if (!buf[token_off]) {
							continue;
						}
						cur_db = sdb_ns (cur_db, buf + token_off, 1);
						if (!cur_db) {
							cur_db = s;
							break;
						}
					}
					ls_destroy (path);
					break;
				}
				case STATE_VALUE:
					if (!*buf || !buf[token_begin]) {
						break;
					}
					sdb_set (cur_db, buf, buf + token_begin, 0);
					break;
				default:
					break;
				}
				if (buf_filled && pos < buf_filled - 1) {
					memmove (buf, buf + pos + 1, buf_filled - pos - 1);
				}
				// prepare for next line
				shift = 0;
				if (pos + 1 <= buf_filled) {
					buf_filled -= pos + 1;
				} else {
					// special case, when at the end of the file
					// here pos == buf_filled
					buf_filled = 0;
				}
				pos = 0;
				state = STATE_NEWLINE;
				continue;
			} else {
				if (unescape) {
					if (buf[pos] == 'n') {
						buf[pos - shift] = '\n';
					} else {
						buf[pos - shift] = buf[pos];
					}
					unescape = false;
				} else if (buf[pos] == '\\') {
					// got a backslash, the next char, unescape in the next iteration or die!
					shift++;
					unescape = true;
				} else if (state == STATE_PATH && buf[pos] == '/') {
					// new path token
					buf[pos - shift] = '\0';
					ls_push (path, (void *)token_begin);
					token_begin = pos + 1;
					shift = 0;
				} else if (state == STATE_KEY && buf[pos] == '=') {
					// switch from key into value mode
					buf[pos - shift] = '\0';
					token_begin = pos + 1;
					shift = 0;
					state = STATE_VALUE;
				} else if (shift) {
					// just some char, shift it back if necessary
					buf[pos - shift] = buf[pos];
				}
			}
			pos++;
		}
		if (!read) {
			break;
		}
	}
	free (buf);
	return ret;
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

