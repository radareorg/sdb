/* sdb - MIT - Copyright 2020 - thestr4ng3r */

#include "sdb.h"

#include <fcntl.h>
#include <limits.h>

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
 * These paths are always absolute from the root. Characters that must be escaped in a path are: '/', '\\', '\n', '\r':
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
 * Other than that, at any postion, '\\', '\n' and '\r' must be escaped:
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
// flush a block of text that doesn't have to be escaped
#define FLUSH do { if (p != n) { write (fd, p, n - p); p = n; } } while (0)
// write and escape a string from str to fd
#define ESCAPE_LOOP(fd, str, escapes) do { \
		const char *p = str; \
		const char *n = p; \
		while (*n) { \
			switch (*n) { escapes } \
			n++; \
		} \
		FLUSH; \
	} while (0)
#define ESCAPE(c, repl, replsz) \
		case c: \
			FLUSH; \
			p++; \
			write (fd, "\\"repl, replsz + 1); \
			break;

static void write_path(int fd, SdbList *path) {
	write (fd, "/", 1); // always print a /, even if path is empty
	SdbListIter *it;
	const char *path_token;
	bool first = true;
	ls_foreach (path, it, path_token) {
		if (first) {
			first = false;
		} else {
			write (fd, "/", 1);
		}
		ESCAPE_LOOP (fd, path_token,
			ESCAPE ('\\', "\\", 1);
			ESCAPE ('/', "/", 1);
			ESCAPE ('\n', "n", 1);
			ESCAPE ('\r', "r", 1);
		);
	}
}

static void write_key(int fd, const char *k) {
	// escape leading '/'
	if (*k == '/') {
		write (fd, "\\", 1);
	}
	ESCAPE_LOOP (fd, k,
		ESCAPE ('\\', "\\", 1);
		ESCAPE ('=', "=", 1);
		ESCAPE ('\n', "n", 1);
		ESCAPE ('\r', "r", 1);
	);
}

static void write_value(int fd, const char *v) {
	ESCAPE_LOOP (fd, v,
		ESCAPE ('\\', "\\", 1);
		ESCAPE ('\n', "n", 1);
		ESCAPE ('\r', "r", 1);
	);
}

static bool text_save(Sdb *s, int fd, bool sort, SdbList *path) {
	// path
	write_path (fd, path);
	write (fd, "\n", 1);

	// k=v entries
	SdbList *l = sdb_foreach_list (s, sort);
	SdbKv *kv;
	SdbListIter *it;
	ls_foreach (l, it, kv) {
		const char *k = sdbkv_key (kv);
		const char *v = sdbkv_value (kv);

		write_key (fd, k);
		write (fd, "=", 1);
		write_value (fd, v);

		write (fd, "\n", 1);
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
		write (fd, "\n", 1);
		ls_push (path, ns->name);
		text_save (ns->sdb, fd, sort, path);
		ls_pop (path);
	}
	if (l != s->ns) {
		ls_free (l);
	}

	return true;
}
#undef FLUSH
#undef ESCAPE_LOOP
#undef ESCAPE

SDB_API bool sdb_text_save_fd(Sdb *s, int fd, bool sort) {
	SdbList *path = ls_new ();
	if (!path) {
		return false;
	}
	bool r = text_save (s, fd, sort, path);
	ls_free (path);
	return r;
}

SDB_API bool sdb_text_save(Sdb *s, const char *file, bool sort) {
	int fd = open (file, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		return false;
	}
	bool r = sdb_text_save_fd (s, fd, sort);
	close (fd);
	return r;
}

typedef enum {
	STATE_NEWLINE,
	STATE_PATH,
	STATE_KEY,
	STATE_VALUE
} LoadState;

typedef struct {
	int fd;
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
	LoadState state;
	bool unescape; // whether the prev char was a backslash, i.e. the current one is escaped
} LoadCtx;


// load more data from ctx->f into ctx->buf starting at ctx->pos
// and update ctx->buf_filled and ctx->eof.
// may realloc and change ctx->buf and ctx->bufsz.
static bool load_read_more(LoadCtx *ctx) {
	// realloc if no space
	if (ctx->pos >= ctx->bufsz - 1) {
		size_t newsz = ctx->bufsz + ctx->bufsz;
		if (newsz < ctx->bufsz || newsz > (size_t)INT_MAX) {
			return false;
		}
		char *newbuf = realloc (ctx->buf, newsz);
		if (!newbuf) {
			return false;
		}
		ctx->buf = newbuf;
		ctx->bufsz = newsz;
	}
	// read full buffer (leave 1 for null-terminating)
	int red = read (ctx->fd, ctx->buf + ctx->pos, ctx->bufsz - 1 - ctx->pos);
	if (red < 0) {
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
	if (ctx->buf[ctx->pos] == '\n' || ctx->buf[ctx->pos] == '\r') {
		load_flush_line (ctx);
		return;
	}

	if (ctx->state == STATE_NEWLINE) {
		// at the start of a line, decide whether it's a path or a k=v
		// by whether there is a leading slash.
		if (ctx->buf[ctx->pos] == '/') {
			ctx->state = STATE_PATH;
			ctx->token_begin = 1;
			ctx->pos++;
			return;
		}
		ctx->state = STATE_KEY;
	}

	if (ctx->unescape) {
		char raw_char;
		switch (ctx->buf[ctx->pos]) {
		case 'n':
			raw_char = '\n';
			break;
		case 'r':
			raw_char = '\r';
			break;
		default:
			raw_char = ctx->buf[ctx->pos];
			break;
		}
		ctx->buf[ctx->pos - ctx->shift] = raw_char;
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

SDB_API bool sdb_text_load_fd(Sdb *s, int fd) {
	LoadCtx ctx = {
		.fd = fd,
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
	for (;;) {
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
			load_process_single_char (&ctx);
		}
	}
	free (ctx.buf);
	ls_free (ctx.path);
	return ret;
}

SDB_API bool sdb_text_load(Sdb *s, const char *file) {
	int fd = open (file, O_RDONLY);
	if (fd < 0) {
		return false;
	}
	bool r = sdb_text_load_fd (s, fd);
	close (fd);
	return r;
}

