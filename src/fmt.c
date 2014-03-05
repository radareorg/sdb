/* sdb - LGPLv3 - Copyright 2014 - pancake */

#if 0
 TODO
 ====
 - This is just a quick implementation

#endif

#include "sdb.h"

// SLOW CONCAT
#define out_concat(x) if (x) { \
	char *o =(void*)realloc((void*)out, 2+strlen(x)+(out?strlen(out):0)); \
	if (o) { if (*o) strcat (o, ","); out=o; strcat (o, x); } \
}

SDB_API char *sdb_fmt_tostr(void *stru, const char *fmt) {
	char *out = NULL;
	ut8 *p = stru;
	char buf[128];
	int n, len = 0;

	for (; *fmt; fmt++) {
		n = 4;
		switch (*fmt) {
		case 'b': 
			snprintf (buf, sizeof (buf), "%d", *((ut8*)(p+len)));
			out_concat (buf); break;
		case 'h': 
			snprintf (buf, sizeof (buf), "%d", *((short*)(p+len)));
			out_concat (buf); break;
		case 'd': snprintf (buf, sizeof (buf), "%u", *((int*)(p+len)));
			out_concat (buf); break;
		case 'q': eprintf ("FMT: q: todo\n"); n = 8; break;
		case 's': out_concat (*((char**)(p+len))); break;
		case 'p': eprintf ("FMT: p: todo\n"); break;
		}
		len += R_MAX (sizeof (void*), n); // align
	}
	return out;
}

// TODO: return false if array length != fmt length
SDB_API int sdb_fmt_tobin(const char *_str, const char *fmt, void *stru) {
	int nxt, n, idx = 0;
	char *str, *ptr, *word;
	str = ptr = strdup (_str);
	for (; *fmt; fmt++) {
		word = sdb_array_string (ptr, &nxt);
		if (!word || !*word)
			break;
		n = 4; // ALIGN
		switch (*fmt) {
		case 'q': *((ut64*)(stru + idx)) = sdb_atoi (word); n=8; break;
		case 'h': *((short*)(stru + idx)) = (short)sdb_atoi (word); break;
		case 'd': *((int*)(stru + idx)) = (int)sdb_atoi (word); break;
		case 's': *((char**)(stru + idx)) = strdup (word); break;
		case 'p': *((void**)(stru + idx)) = (void*)(size_t)sdb_atoi (word); break;
		default: eprintf ("WTF\n"); break;
		}
		idx += R_MAX(sizeof (void*), n); // align
		if (!nxt)
			break;
		ptr = (char*)sdb_array_next (word);
	}
	return 1;
}

SDB_API void sdb_fmt_free (void *p, const char *fmt) {
	// TODO: free() 's' and memset the rest
}

SDB_API int sdb_fmt_init (void *p, const char *fmt) {
	int len = 0;
	for (; *fmt; fmt++) {
		switch (*fmt) {
		case 'b': len += sizeof (ut8); break;   // 1
		case 'h': len += sizeof (short); break; // 2
		case 'd': len += sizeof (ut32); break;  // 4
		case 'q': len += sizeof (ut64); break;  // 8
		case 's': len += sizeof (char*); break; // void*
		case 'p': len += sizeof (char*); break; // void *
		}
	}
	if (p) memset (p, 0, len);
	return len;
}


#if 0
main() {
	#define STRUCT_PERSON_FORMAT "ds"
	#define STRUCT_PERSON_SIZE sizeof (struct Person)
	typedef struct person {
		int foo;
		char *str;
	} Person;

	Person p;

	sdb_fmt_init (&p, "ds");
	sdb_fmt_tobin ("123,bar", "ds", &p);
	eprintf ("--> %d,%s\n", p.foo, p.str);

	{
		char *o = sdb_fmt_tostr (&p, "ds");
		eprintf ("%s\n", o);
		free (o);
	}
	sdb_fmt_free (&p, "ds");
}
#endif
