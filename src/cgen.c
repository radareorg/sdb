#include <sdb/sdb.h>

SDB_API char *sdb_cgen_header(const char *cname, bool textmode) {
	StrBuf *sb = strbuf_new ();
	if (!sb) {
		return NULL;
	}
	if (textmode) {
		strbuf_append (sb, "// SDB-CGEN V" SDB_VERSION "\n"
			"#include <ctype.h>\n"
			"#include <stdio.h>\n"
			"#include <string.h>\n"
			"\n"
			"struct kv { const char *name; const char *value; };\n"
			"static const struct kv kvs[] = {", 1);
		strbuf_appendf (sb, 1, "// gcc -DMAIN=1 %s.c ; ./a.out > %s.h", cname, cname);
	} else {
		strbuf_append (sb, "%{\n"
			"#include <stdio.h>\n"
			"#include <string.h>\n"
			"#include <ctype.h>\n"
			"%}\n"
			"\n"
			"struct kv { const char *name; const char *value; };\n"
			"%%", 1);
		strbuf_appendf (sb, 1, "// gperf -aclEDCIG --null-strings -H sdb_hash_c_%s -N sdb_get_c_%s -t %s.gperf > %s.c", 
				cname, cname, cname, cname);
		strbuf_appendf (sb, 1, "// gcc -DMAIN=1 %s.c ; ./a.out > %s.h", cname, cname);
	}
	return strbuf_drain (sb);
}

// TODO rename gperf with cgen
SDB_API char *sdb_cgen_footer(const char *name, const char *cname, bool textmode) {
	StrBuf *sb = strbuf_new ();
	if (!sb) {
		return NULL;
	}
	if (textmode) {
		strbuf_append (sb, "  {NULL, NULL}\n"
			"};\n"
			// "// TODO\n"
			"typedef int (*GperfForeachCallback)(void *user, const char *k, const char *v);", 1);
		// strbuf_appendf (sb, 1, "// %p", cname);
		strbuf_appendf (sb, 1, "int gperf_%s_foreach(GperfForeachCallback cb, void *user) {", cname);
		strbuf_append (sb, "  int i = 0; while (kvs[i].name) {\n"
			"  cb (user, kvs[i].name, kvs[i].value);\n"
			"  i++;}\n"
			"  return 0;\n"
			"}", 1);
		strbuf_appendf (sb, 1, "const char *gperf_%s_get(const char *s) {", cname);
		strbuf_append (sb, "  int i = 0; while (kvs[i].name) {\n"
			"  if (!strcmp (s, kvs[i].name)) return kvs[i].value;\n"
			"  i++;}\n"
			"  return NULL;\n"
			"}", 1);
		strbuf_appendf (sb, 1, "#define sdb_hash_c_%s(x,y) gperf_%s_hash(x)", cname, cname);
		strbuf_appendf (sb, 1, "const unsigned int gperf_%s_hash(const char *s) {", cname);
		strbuf_append (sb, "  int sum = strlen (s);\n"
			"  while (*s) { sum += *s; s++; }\n"
			"  return sum;\n"
			"}", 1);
		
		strbuf_appendf (sb, 1,
			"struct {const char *name;void *get;void *hash;void *foreach;} gperf_%s = {\n"
			"  .name = \"%s\",\n"
			"  .get = &gperf_%s_get,\n"
			"  .hash = &gperf_%s_hash,\n"
			"  .foreach = &gperf_%s_foreach\n"
			"};\n"
			"\n"
			"#if MAIN\n"
			"int main () {\n"
			"	const char *s = ((char*(*)(char*))gperf_%s.get)(\"foo\");\n"
			"	printf (\"%%s\\n\", s);\n"
			"}\n"
			"#endif", cname, name, cname, cname, cname, cname);
		
		return strbuf_drain (sb);
	}
	strbuf_append (sb, "%%\n"
		"// SDB-CGEN V" SDB_VERSION "\n"
		"typedef int (*GperfForeachCallback)(void *user, const char *k, const char *v);", 1);
	// strbuf_appendf (sb, 1, "// %p", cname);
	strbuf_appendf (sb, 1, "int gperf_%s_foreach(GperfForeachCallback cb, void *user) {", cname);
	strbuf_append (sb, "\tint i;for (i=0;i<TOTAL_KEYWORDS;i++) {\n"
		"\tconst struct kv *w = &wordlist[i];\n"
		"\tif (!cb (user, w->name, w->value)) return 0;\n"
		"}\n"
		"return 1;}", 1);
	strbuf_appendf (sb, 1, "const char* gperf_%s_get(const char *s) {", cname);
	strbuf_appendf (sb, 1, "\tconst struct kv *o = sdb_get_c_%s (s, strlen(s));", cname);
	strbuf_append (sb, "\treturn o? o->value: NULL;\n"
		"}", 1);
	strbuf_appendf (sb, 1, "const unsigned int gperf_%s_hash(const char *s) {", cname);
	strbuf_appendf (sb, 1, "\treturn sdb_hash_c_%s(s, strlen (s));", cname);
	strbuf_append (sb, "}", 1);
	
	strbuf_appendf (sb, 1, 
		"struct {const char*name;void*get;void*hash;void *foreach;} gperf_%s = {\n"
		"\t.name = \"%s\",\n"
		"\t.get = &gperf_%s_get,\n"
		"\t.hash = &gperf_%s_hash,\n"
		"\t.foreach = &gperf_%s_foreach\n"
		"};\n"
		"\n"
		"#if MAIN\n"
		"int main () {\n"
		"	char line[1024];\n"
		"	FILE *fd = fopen (\"%s.gperf\", \"r\");\n"
		"	if (!fd) {\n"
		"		fprintf (stderr, \"Cannot open %s.gperf\\n\");\n"
		"		return 1;\n"
		"	}\n"
		"	int mode = 0;\n"
		"	printf (\"#ifndef INCLUDE_%s_H\\n\");\n"
		"	printf (\"#define INCLUDE_%s_H 1\\n\");\n"
		"	while (!feof (fd)) {\n"
		"		*line = 0;\n"
		"		fgets (line, sizeof (line), fd);\n"
		"		if (mode == 1) {\n"
		"			char *comma = strchr (line, ',');\n"
		"			if (comma) {\n"
		"				*comma = 0;\n"
		"				char *up = sdb_strdup (line);\n"
		"				char *p = up; while (*p) { *p = toupper (*p); p++; }\n"
		"				printf (\"#define GPERF_%s_%%s %%d\\n\",\n"
		"					line, sdb_hash_c_%s (line, comma - line));\n"
		"			}\n"
		"		}\n"
		"		if (*line == '%%' && line[1] == '%%') { mode++; }\n"
		"	}\n"
		"	printf (\"#endif\\n\");\n"
		"}\n"
		"#endif",
		cname, name, cname, cname, cname, name, name, cname, cname, cname, cname);
		
	strbuf_append (sb, "", 1);
	return strbuf_drain (sb);
}

