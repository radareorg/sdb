/* sdb - MIT - Copyright 2011-2026 - pancake */

#include <fcntl.h>
#include <ctype.h>
#include "sdb_private.h"

#define NEWLINE_AFTER_QUERY 1

SDB_API bool sdb_queryf(Sdb *s, const char *fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	char *cmd = sdb_vstrdupf (fmt, ap);
	va_end (ap);
	int ret = cmd? sdb_query (s, cmd): 0;
	sdb_gh_free (cmd);
	return ret;
}

SDB_API char *sdb_querysf(Sdb *s, char *buf, size_t buflen, const char *fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	char *cmd = sdb_vstrdupf (fmt, ap);
	va_end (ap);
	char *ret = cmd? sdb_querys (s, buf, buflen, cmd): NULL;
	sdb_gh_free (cmd);
	return ret;
}

// TODO: Reimplement as a function with optimized concat
#define out_concat(x) if ((x) && *(x)) { \
	strbuf_append (out, x, 1); \
}

typedef struct {
	StrBuf *out;
	bool encode;
	char *root;
} ForeachListUser;

static bool foreach_list_cb(void *user, const char *k, const char *v) {
	ForeachListUser *rlu = (ForeachListUser*)user;
	char *line = NULL;
	char *root = NULL;
	int rlen, klen, vlen;
	ut8 *v2 = NULL;
	if (!rlu) {
		return false;
	}
	root = rlu->root;
	klen = strlen (k);
	if (rlu->encode) {
		v2 = sdb_decode (v, NULL);
		if (v2) {
			v = (const char *)v2;
		}
	}
	vlen = strlen (v);
	if (root) {
		rlen = strlen (root);
		line = (char *)sdb_gh_malloc (klen + vlen + rlen + 3);
		if (!line) {
			sdb_gh_free (v2);
			return false;
		}
		memcpy (line, root, rlen);
		line[rlen] = '/'; /*append the '/' at the end of the namespace */
		memcpy (line + rlen + 1, k, klen);
		line[rlen + klen + 1] = '=';
		memcpy (line + rlen + klen + 2, v, vlen + 1);
	} else {
		line = (char *)sdb_gh_malloc (klen + vlen + 2);
		if (!line) {
			sdb_gh_free (v2);
			return false;
		}
		memcpy (line, k, klen);
		line[klen] = '=';
		memcpy (line + klen + 1, v, vlen + 1);
	}
	strbuf_append (rlu->out, line, 1);
	sdb_gh_free (v2);
	sdb_gh_free (line);
	return true;
}

static void walk_namespace(StrBuf *sb, char *root, int left, char *p, SdbNs *ns, bool encode) {
	int len;
	SdbListIter *it;
	SdbNs *n;
	ForeachListUser user = { sb, encode, root };
	char *roote = root + strlen (root);
	if (!ns->sdb) {
		return;
	}
	/*Pick all key=value in the local ns*/
	sdb_foreach (ns->sdb, foreach_list_cb, &user);

	/*Pick "sub"-ns*/
	ls_foreach_cast (ns->sdb->ns, it, SdbNs*, n) {
		len = strlen (n->name);
		p[0] = '/';
		if (len + 2 < left) {
			memcpy (p + 1, n->name, len + 1);
			left -= len + 2;
		}
		walk_namespace (sb, root, left, roote + len + 1, n, encode);
	}
}

typedef enum {
	Q_NEXT = 0, // continue with the next ;-separated segment
	Q_STOP,     // stop processing and return the collected output
	Q_ERR,      // stop processing and return NULL
} QStatus;

typedef struct {
	Sdb *s;          // namespace the segment operates on
	StrBuf *out;
	bool encode;     // '%' prefix: encode values on write, decode on read
	char *cmd;       // key expression, mutated in place
	const char *p;   // array key: the text after the ']' (cmd when no brackets)
	const char *val; // value after the '=' or NULL
	char *eq;        // byte where the '=' was cut, doubles as has-value flag
	char *json;      // ':' json path split point or NULL
} Query;

static void q_puts(Query *q, const char *s) {
	if (s && *s) {
		strbuf_append (q->out, s, 1);
	}
}

// *, ** and *** dumps; when matched they terminate the query
static bool q_dump(Query *q) {
	SdbListIter *it;
	SdbNs *ns;
	if (!strcmp (q->cmd, "***")) {
		char root[1024];
		ls_foreach_cast (q->s->ns, it, SdbNs*, ns) {
			int name_len = strlen (ns->name);
			if (name_len < (long)sizeof (root)) {
				memcpy (root, ns->name, name_len + 1);
				walk_namespace (q->out, root, sizeof (root) - name_len,
					root + name_len, ns, q->encode);
			}
		}
		return true;
	}
	if (!strcmp (q->cmd, "**")) {
		ls_foreach_cast (q->s->ns, it, SdbNs*, ns) {
			q_puts (q, ns->name);
		}
		return true;
	}
	if (!strcmp (q->cmd, "*")) {
		ForeachListUser user = { q->out, q->encode, NULL };
		SdbList *list = sdb_foreach_list (q->s, true);
		SdbListIter *iter;
		SdbKv *kv;
		ls_foreach_cast (list, iter, SdbKv*, kv) {
			foreach_list_cb (&user, sdbkv_key (kv), sdbkv_value (kv));
		}
		ls_free (list);
		return true;
	}
	return false;
}

// ~key deletes the matching keys, ~~expr greps
static void q_grep(Query *q) {
	if (q->cmd[1] == '~') {
		SdbKv *kv;
		SdbListIter *li;
		SdbList *l = sdb_foreach_match (q->s, q->cmd + 2, false);
		ls_foreach_cast (l, li, SdbKv*, kv) {
			strbuf_append (q->out, sdbkv_key (kv), 0);
			strbuf_append (q->out, "=", 0);
			strbuf_append (q->out, sdbkv_value (kv), 1);
		}
		ls_free (l);
	} else {
		sdb_unset_like (q->s, q->cmd + 1);
	}
}

// +key/-key increments or decrements numbers, concats/uncats strings
static QStatus q_incdec(Query *q) {
	char *cmd = q->cmd;
	char numstr[64];
	if (cmd[1] == '[') { // +[idx]key[=n]  -->  key[idx] += n or 1
		const char *eb = strchr (cmd, ']');
		if (!eb) {
			return Q_STOP; // missing ']'
		}
		int idx = sdb_atoi (cmd + 2);
		ut64 curnum = sdb_array_get_num (q->s, eb + 1, idx, 0);
		if (q->eq) {
			st64 neq = sdb_atoi (q->eq);
			curnum += (*cmd == '+')? neq: -neq;
			sdb_array_set_num (q->s, eb + 1, idx, curnum, 0);
		} else {
			curnum += (*cmd == '+')? 1: -1;
			q_puts (q, sdb_itoa (curnum, 10, numstr, sizeof (numstr)));
		}
		return Q_NEXT;
	}
	if (q->val) { // +key=n / +key=str
		const char *val = q->val;
		if (sdb_isnum (val)) {
			int d, op = *cmd;
			if (*val == '-') { // adding a negative is subtracting
				op = (*cmd == '-')? '+': '-';
				d = sdb_atoi (val + 1);
			} else {
				d = sdb_atoi (val);
			}
			if (op == '+') {
				sdb_num_inc (q->s, cmd + 1, d, 0);
			} else {
				sdb_num_dec (q->s, cmd + 1, d, 0);
			}
		} else if (*cmd == '+') {
			sdb_concat (q->s, cmd + 1, val, 0);
		} else {
			sdb_uncat (q->s, cmd + 1, val, 0);
		}
		return Q_NEXT;
	}
	// +key / +key:jspath, step by one and print the result in the same base
	int base = sdb_num_base (sdb_const_get (q->s, cmd + 1, 0));
	ut64 n;
	if (q->json) {
		base = 10; // json is base10 only
		*q->json = 0;
		n = (*cmd == '+')
			? sdb_json_num_inc (q->s, cmd + 1, q->json + 1, 1, 0)
			: sdb_json_num_dec (q->s, cmd + 1, q->json + 1, 1, 0);
		*q->json = ':';
	} else {
		n = (*cmd == '+')
			? sdb_num_inc (q->s, cmd + 1, 1, 0)
			: sdb_num_dec (q->s, cmd + 1, 1, 0);
	}
	if (base == 16) {
		snprintf (numstr, sizeof (numstr), "0x%" PRIx64, n);
	} else {
		snprintf (numstr, sizeof (numstr), "%" PRId64, n);
	}
	q_puts (q, numstr);
	return Q_NEXT;
}

// [+..]key / [-..]key: stacks, head/tail shifts and signed indexes
static QStatus q_array_pm(Query *q) {
	Sdb *s = q->s;
	char *cmd = q->cmd;
	const char *p = q->p;
	if (cmd[1] == cmd[2]) { // [++]key=v push, [--]key pop
		if (q->eq && cmd[1] == '+') {
			sdb_array_push (s, p, q->val, 0);
		} else if (!q->eq && cmd[1] == '-') {
			char *ret = sdb_array_pop (s, p, 0);
			q_puts (q, ret);
			sdb_gh_free (ret);
		} // else invalid syntax
		return Q_NEXT;
	}
	if (!cmd[2]) {
		if (q->eq) {
			if (cmd[1] == '+') { // [+]key=v - add if not present
				sdb_array_add (s, p, q->val, 0);
			} else { // [-]key=v - remove value
				sdb_array_remove (s, p, q->val, 0);
			}
		} else { // [+]key / [-]key - print and drop first/last element
			int idx = (cmd[1] == '+')? 0: -1;
			char *ret = sdb_array_get (s, p, idx, 0);
			q_puts (q, ret);
			sdb_array_delete (s, p, idx, 0);
			sdb_gh_free (ret);
		}
		return Q_NEXT;
	}
	int i = atoi (cmd + 1);
	if (q->eq) {
		if (i < 0) { // [-n]key=v - print and delete element n
			char *arr = sdb_array_get (s, p, -i, NULL);
			if (!arr) {
				return Q_STOP;
			}
			if (q->encode) {
				char *dec = (char *)sdb_decode (arr, NULL);
				sdb_gh_free (arr);
				if (!dec) {
					return Q_STOP;
				}
				arr = dec;
			}
			q_puts (q, arr);
			sdb_array_delete (s, p, -i, 0);
			sdb_gh_free (arr);
			return Q_NEXT;
		}
		char *sval = (char *)q->val; // [+n]key=v insert, [n]key=v set
		if (q->encode) {
			sval = (char *)sdb_encode ((const ut8 *)q->val, -1);
		}
		if (cmd[1] == '+') {
			sdb_array_insert (s, p, i, sval, 0);
		} else {
			sdb_array_set (s, p, i, sval, 0);
		}
		if (q->encode) {
			sdb_gh_free (sval);
		}
		return Q_NEXT;
	}
	if (i == 0) {
		if (cmd[1] == '-') { // [-v]key - remove value v
			sdb_array_remove (s, p, cmd + 2, 0);
		} // else TODO: [v]key - get index of value v
		return Q_NEXT;
	}
	char *arr = sdb_array_get (s, p, (i < 0)? -i: i, NULL);
	if (arr && *arr) {
		q_puts (q, arr);
		if (i < 0) { // [-n]key - print and delete element n
			sdb_array_delete (s, p, -i, 0);
		}
	}
	sdb_gh_free (arr);
	return Q_NEXT;
}

// [...]key array operations
static QStatus q_array(Query *q) {
	char *cmd = q->cmd;
	if (cmd[1] == '?') { // [?]key - number of elements
		char numstr[32];
		snprintf (numstr, sizeof (numstr), "%d", sdb_array_length (q->s, q->p));
		q_puts (q, numstr);
		return Q_NEXT;
	}
	if (cmd[1] == '!') {
		if (cmd[2] == '+') { // [!+]key=v - add sorted
			sdb_array_add_sorted (q->s, q->p, q->val, 0);
		} else { // [!]key - sort
			sdb_array_sort (q->s, q->p, 0);
		}
		return Q_NEXT;
	}
	if (cmd[1] == '#') {
		if (cmd[2] == '+') { // [#+]key=n - add sorted numeric
			sdb_array_add_sorted_num (q->s, q->p, sdb_atoi (q->val), 0);
		} else { // [#]key - sort numeric
			sdb_array_sort_num (q->s, q->p, 0);
		}
		return Q_NEXT;
	}
	if (cmd[1] == '+' || cmd[1] == '-') {
		return q_array_pm (q);
	}
	if (q->eq) { // [n]key=v / []key=v
		char *sval = (char *)q->val;
		if (q->encode) {
			sval = (char *)sdb_encode ((const ut8 *)q->val, -1);
		}
		if (cmd[1]) {
			sdb_array_set (q->s, q->p, atoi (cmd + 1), sval, 0);
			if (q->encode) {
				sdb_gh_free (sval);
			}
		} else if (q->encode) {
			sdb_set_owned (q->s, q->p, sval, 0);
		} else {
			sdb_set (q->s, q->p, sval, 0);
		}
		return Q_NEXT;
	}
	if (cmd[1]) { // [n]key - print element n
		char *el = sdb_array_get (q->s, q->p, atoi (cmd + 1), NULL);
		if (q->encode) {
			char *dec = (char *)sdb_decode (el, NULL);
			if (dec) {
				sdb_gh_free (el);
				el = dec;
			}
		}
		q_puts (q, el);
		sdb_gh_free (el);
		return Q_NEXT;
	}
	// []key - print the whole array, one element per line
	const char *sval = sdb_const_get (q->s, q->p, 0);
	if (!sval) {
		return Q_STOP;
	}
	char *line = sdb_strdup (sval);
	if (!line) {
		return Q_ERR;
	}
	size_t i;
	for (i = 0; line[i]; i++) {
		if (line[i] == SDB_RS && line[i + 1]) {
			line[i] = '\n';
		}
	}
	if (q->encode) {
		char *dec = (char *)sdb_decode (line, NULL);
		if (dec) {
			sdb_gh_free (line);
			line = dec;
		}
	}
	q_puts (q, line);
	sdb_gh_free (line);
	return Q_NEXT;
}

// plain and json get/set
static QStatus q_kv(Query *q) {
	char *cmd = q->cmd;
	char *json = q->json;
	if (q->eq) { // key=v / key:jspath=v
		const char *val = q->val;
		char *enc = NULL;
		if (q->encode) {
			enc = (char *)sdb_encode ((const ut8 *)val, -1);
			val = enc;
		}
		if (json > q->eq) {
			json = NULL; // the ':' belongs to the value
		}
		if (json) {
			*json++ = 0;
			sdb_json_set (q->s, cmd, json, val, 0);
		} else {
			while (*val && isspace (*val)) {
				val++;
			}
			if (*cmd) {
				int clen = strlen (cmd) - 1;
				while (clen >= 0 && isspace (cmd[clen])) {
					cmd[clen] = '\0';
					clen--;
				}
				sdb_set (q->s, cmd, val, 0);
			}
		}
		sdb_gh_free (enc);
		return Q_NEXT;
	}
	if (json) { // key:jspath - print a json field, key: - print indented
		*json++ = 0;
		if (*json) {
			char *js = sdb_json_get (q->s, cmd, json, 0);
			if (js) {
				if (q->encode) {
					char *dec = (char *)sdb_decode (js, NULL);
					sdb_gh_free (js);
					if (!dec) {
						return Q_STOP;
					}
					js = dec;
				}
				q_puts (q, js);
				sdb_gh_free (js);
			}
		} else {
			char *o = sdb_json_indent (sdb_const_get (q->s, cmd, 0), "  ");
			q_puts (q, o);
			sdb_gh_free (o);
		}
		return Q_NEXT;
	}
	const char *v = sdb_const_get (q->s, cmd, 0);
	if (v) {
		if (q->encode) {
			char *dec = (char *)sdb_decode (v, NULL);
			q_puts (q, dec);
			sdb_gh_free (dec);
		} else {
			q_puts (q, v);
		}
	}
	return Q_NEXT;
}

// parse one ;-separated segment into q and dispatch it
static QStatus q_segment(Query *q, char *cmd, char **pnext, char **pnewcmd) {
	while (*cmd == ' ' || *cmd == '\t') {
		cmd++;
	}
	char *p = cmd;
	if (*p == '#') { // #text - print the sdb hash of text
		char numstr[32];
		char *next = strchr (p + 1, ';');
		if (next) {
			*next = 0;
		}
		snprintf (numstr, sizeof (numstr), "0x%08x\n", sdb_hash (p + 1));
		strbuf_append (q->out, numstr, 1);
		if (next) {
			*next = ';';
		}
		*pnext = next;
		return Q_NEXT;
	}
	if (*p == '%') { // %key - encode values on write, decode on read
		q->encode = true;
		cmd++;
		p++;
	}
	bool is_ref = false;
	char *next = NULL;
	const char *val = NULL;
	char *eq = strchr (p, '=');
	if (eq) {
		*eq++ = 0;
		if (*eq == '$') { // key=$otherkey - use the value of otherkey
			next = strchr (eq + 1, ';');
			if (next) {
				*next = 0;
			}
			val = sdb_const_get (q->s, eq + 1, 0);
			if (!val) {
				return Q_STOP; // unknown reference
			}
			if (next) {
				*next = ';';
			}
			is_ref = true; // val lives in the db, not in the cmd buffer
		} else {
			val = eq;
		}
	}
	if (!is_ref) {
		next = (char *)strchr (val? val: cmd, ';');
		if (val && *val == '"') { // key="quoted value"
			char *quot = (char *)++val;
			for (;;) { // find the closing quote, dropping \" escapes
				quot = strchr (quot, '"');
				if (!quot) {
					return Q_ERR; // missing closing quote
				}
				if (quot == val || quot[-1] != '\\') {
					break;
				}
				memmove (quot - 1, quot, strlen (quot) + 1);
			}
			*quot++ = 0;
			next = strchr (quot, ';');
		}
	}
	if (next) {
		*next = 0;
	}
	*pnext = next;
	char *slash = strchr (cmd, '/');
	while (slash) { // walk the namespace path, creating it when writing
		*slash = 0;
		q->s = sdb_ns (q->s, cmd, eq? 1: 0);
		if (!q->s) {
			return Q_ERR; // unknown namespace
		}
		cmd = slash + 1;
		slash = strchr (cmd, '/');
	}
	q->cmd = cmd;
	q->p = cmd;
	q->val = val;
	q->eq = eq;
	if (*cmd == '?') { // ?key - print the value type (then continue as a key)
		q_puts (q, sdb_type (sdb_const_get (q->s, cmd + 1, 0)));
	} else if (*cmd == '*' && q_dump (q)) {
		return Q_STOP;
	}
	q->json = strchr (cmd, ':');
	if (*cmd == '[') { // split "[spec]key" into cmd spec and p key
		char *tp = strchr (cmd, ']');
		if (!tp) {
			return Q_STOP; // missing ']'
		}
		*tp++ = 0;
		q->p = tp;
	} else if (*cmd == '$') { // $key - indirection, key holds the actual key
		char *nc = sdb_get (q->s, cmd + 1, 0);
		*pnewcmd = nc? nc: sdb_strdup ("");
		if (!*pnewcmd) {
			return Q_ERR;
		}
		cmd = q->cmd = *pnewcmd;
		q->p = cmd;
	}
	if (*cmd == '.') { // .file - run queries from a file
		if (q->s->options & SDB_OPTION_FS) {
			if (!sdb_query_file (q->s, cmd + 1)) {
				return Q_STOP; // cannot open file
			}
		}
		return Q_NEXT;
	}
	if (*cmd == '~') {
		q_grep (q);
		return Q_NEXT;
	}
	if (*cmd == '+' || *cmd == '-') {
		return q_incdec (q);
	}
	if (*cmd == '[') {
		return q_array (q);
	}
	return q_kv (q);
}

SDB_API char *sdb_querys(Sdb *r, char *buf, size_t len, const char *_cmd) {
	(void)len; // buf is only the query input when _cmd is NULL, never scratch
	if (!r || (!_cmd && !buf)) {
		return NULL;
	}
	StrBuf *out = strbuf_new ();
	if (!out) {
		return NULL;
	}
	char *original_cmd = NULL;
	char *cmd = buf;
	if (_cmd) {
		cmd = original_cmd = sdb_strdup (_cmd);
		if (!cmd) {
			strbuf_free (out);
			return NULL;
		}
	}
	QStatus st = Q_NEXT;
	char *eq = NULL;
	while (cmd) {
		char *next = NULL;
		char *newcmd = NULL;
		Query q = { .s = r, .out = out };
		st = q_segment (&q, cmd, &next, &newcmd);
		eq = q.eq;
		sdb_gh_free (newcmd);
		if (st != Q_NEXT || !next) {
			break;
		}
		cmd = next + 1;
	}
	if (st == Q_NEXT && eq) {
		*--eq = '='; // restore the last cut for buf-as-input callers
	}
	sdb_gh_free (original_cmd);
	if (st == Q_ERR) {
		strbuf_free (out);
		return NULL;
	}
	return strbuf_drain (out);
}

// TODO: should return a string instead, the must_save can be moved outside
SDB_API bool sdb_query(Sdb *s, const char *cmd) {
	char buf[128];
	bool must_save = ((*cmd == '~') || strchr (cmd, '='));
	char *out = sdb_querys (s, buf, sizeof (buf) - 1, cmd);
	if (out) {
		if (*out) {
			fputs (out, stdout);
		}
		if (out != buf) {
			sdb_gh_free (out);
		}
	}
	return must_save;
}

SDB_API bool sdb_query_lines(Sdb *s, const char *cmd) {
	char *o, *p, *op;
	if (!s || !cmd) {
		return 0;
	}
	op = sdb_strdup (cmd);
	if (!op) {
		return 0;
	}
	p = op;
	do {
		o = strchr (p, '\n');
		if (o) {
			*o = 0;
		}
		(void)sdb_query (s, p);
		if (o) {
			p = o + 1;
		}
	} while (o);
	sdb_gh_free (op);
	return 1;
}

static char *slurp(const char *file) {
	if (!file || !*file) {
		return NULL;
	}
	int fd = open (file, O_RDONLY);
	if (fd == -1) {
		return NULL;
	}
	long sz = lseek (fd, 0, SEEK_END);
	if (sz < 0) {
		close (fd);
		return NULL;
	}
	if (lseek (fd, 0, SEEK_SET) == (off_t)-1) {
		close (fd);
		return NULL;
	}
	char *text = (char *)sdb_gh_malloc (sz + 1);
	if (!text) {
		close (fd);
		return NULL;
	}
	int ret = read (fd, text, sz);
	if (ret != sz) {
		sdb_gh_free (text);
		text = NULL;
	} else {
		text[sz] = 0;
	}
	close (fd);
	return text;
}

SDB_API bool sdb_query_file(Sdb *s, const char* file) {
	int ret = 0;
	char *txt = slurp (file);
	if (txt) {
		ret = sdb_query_lines (s, txt);
		sdb_gh_free (txt);
	}
	return ret;
}
