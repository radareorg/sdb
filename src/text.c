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
	SdbListIter *it;
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

typedef struct {
	FILE *f;
	bool eof;
	size_t bufsz;
	char *buf;
	size_t buf_filled; // how many bytes of buf are currently filled with data from the file
	Sdb *root_db;
	Sdb *cur_db; // current namespace, changes when encountering a path line
	// can't use pointers here because buf may be moved on realloc
	size_t pos; // current processing position in the buffer
	size_t token_begin; // beginning of the currently processed token in the buffer
	size_t shift; // amount to shift chars to the left (from unescaping)
	SdbList/*<size_t>*/ *path;
	enum { STATE_NEWLINE, STATE_PATH, STATE_KEY, STATE_VALUE } state;
	bool unescape; // whether the prev char was a backslash, i.e. the current one is escaped
} LoadCtx;

// load more data from ctx->f into ctx->buf starting at ctx->pos
// and update ctx->buf_filled and ctx->eof.
// may realloc and change ctx->buf and ctx->bufsz.
static bool load_read_more(LoadCtx *ctx) {
	// realloc if no space
	if (ctx->pos >= ctx->bufsz - 1) {
		size_t newsz = ctx->bufsz + ctx->bufsz;
		if (newsz < ctx->bufsz) {
			return false;
		}
		ctx->bufsz = newsz;
		char *newbuf = realloc (ctx->buf, ctx->bufsz);
		if (!newbuf) {
			return false;
		}
		ctx->buf = newbuf;
	}
	// read full buffer (leave 1 for null-terminating)
	size_t red = fread (ctx->buf + ctx->pos, 1, ctx->bufsz - 1 - ctx->pos, ctx->f);
	if (!red && !feof (ctx->f)) {
		// error
		return false;
	}
	// process buffer
	ctx->buf_filled = ctx->pos + red;
	ctx->eof = red == 0;
	return true;
}

// to be called at the end of a line.
// save all the data processed from the line into the database.
static void load_flush_line(LoadCtx *ctx) {
	ctx->unescape = false;
	// finish up the line
	ctx->buf[ctx->pos - ctx->shift] = '\0';
	switch (ctx->state) {
	case STATE_PATH: {
		ls_push (ctx->path, (void *)ctx->token_begin);
		// TODO: remove ctx->token_begin = ctx->pos + 1;
		SdbListIter *it;
		void *token_off_tmp;
		ctx->cur_db = ctx->root_db;
		ls_foreach (ctx->path, it, token_off_tmp) {
			size_t token_off = (size_t)token_off_tmp;
			if (!ctx->buf[token_off]) {
				continue;
			}
			ctx->cur_db = sdb_ns (ctx->cur_db, ctx->buf + token_off, 1);
			if (!ctx->cur_db) {
				ctx->cur_db = ctx->root_db;
				break;
			}
		}
		ls_destroy (ctx->path);
		break;
	}
	case STATE_VALUE:
		if (!*ctx->buf || !ctx->buf[ctx->token_begin]) {
			break;
		}
		sdb_set (ctx->cur_db, ctx->buf, ctx->buf + ctx->token_begin, 0);
		break;
	default:
		break;
	}
	if (ctx->buf_filled && ctx->pos < ctx->buf_filled - 1) {
		memmove (ctx->buf, ctx->buf + ctx->pos + 1, ctx->buf_filled - ctx->pos - 1);
	}
	// prepare for next line
	ctx->shift = 0;
	if (ctx->pos + 1 <= ctx->buf_filled) {
		ctx->buf_filled -= ctx->pos + 1;
	} else {
		// special case, when at the end of the file
		// here pos == buf_filled
		ctx->buf_filled = 0;
	}
	ctx->pos = 0;
	ctx->state = STATE_NEWLINE;
}

static void load_process_single_char(LoadCtx *ctx) {
	if (ctx->state == STATE_NEWLINE) {
		// at the start of a line, decide whether it's a path or a k=v
		// by whether there is a leading slash.
		if (ctx->buf[ctx->pos] == '/') {
			ctx->state = STATE_PATH;
			ctx->token_begin = 1;
			ctx->pos++;
			return;
		} else {
			ctx->state = STATE_KEY;
		}
	}

	if (ctx->unescape) {
		if (ctx->buf[ctx->pos] == 'n') {
			ctx->buf[ctx->pos - ctx->shift] = '\n';
		} else {
			ctx->buf[ctx->pos - ctx->shift] = ctx->buf[ctx->pos];
		}
		ctx->unescape = false;
	} else if (ctx->buf[ctx->pos] == '\\') {
		// got a backslash, the next char, unescape in the next iteration or die!
		ctx->shift++;
		ctx->unescape = true;
	} else if (ctx->state == STATE_PATH && ctx->buf[ctx->pos] == '/') {
		// new path token
		ctx->buf[ctx->pos - ctx->shift] = '\0';
		ls_push (ctx->path, (void *)ctx->token_begin);
		ctx->token_begin = ctx->pos + 1;
		ctx->shift = 0;
	} else if (ctx->state == STATE_KEY && ctx->buf[ctx->pos] == '=') {
		// switch from key into value mode
		ctx->buf[ctx->pos - ctx->shift] = '\0';
		ctx->token_begin = ctx->pos + 1;
		ctx->shift = 0;
		ctx->state = STATE_VALUE;
	} else if (ctx->shift) {
		// just some char, shift it back if necessary
		ctx->buf[ctx->pos - ctx->shift] = ctx->buf[ctx->pos];
	}
	ctx->pos++;
}

#define INITIAL_BUFSZ 32

SDB_API bool sdb_text_fload(Sdb *s, FILE *f) {
	LoadCtx ctx = {
		.f = f,
		.eof = false,
		.bufsz = INITIAL_BUFSZ,
		.buf = malloc (INITIAL_BUFSZ),
		.root_db = s,
		.cur_db = s,
		.pos = 0,
		.token_begin = 0,
		.shift = 0,
		.path = ls_new (),
		.state = STATE_NEWLINE,
		.unescape = false
	};
	if (!ctx.buf || !ctx.path) {
		free (ctx.buf);
		ls_free (ctx.path);
		return false;
	}
	bool ret = true;
	while (true) {
		if (!load_read_more (&ctx)) {
			ret = false;
			break;
		}
		// process buffer
		if (ctx.eof) {
			load_flush_line (&ctx);
			break;
		}
		while (ctx.pos < ctx.buf_filled) {
			if (ctx.buf[ctx.pos] == '\n') {
				load_flush_line (&ctx);
				continue;
			}
			load_process_single_char (&ctx);
		}
	}
	free (ctx.buf);
	ls_free (ctx.path);
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

