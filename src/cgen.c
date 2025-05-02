#include <sdb/sdb.h>

SDB_API void sdb_cgen_header(const char *cname, bool textmode) {
	if (textmode) {
		printf ("// SDB-CGEN V" SDB_VERSION "\n");
		printf ("// gcc -DMAIN=1 %s.c ; ./a.out > %s.h\n", cname, cname);
		printf ("#include <ctype.h>\n");
		printf ("#include <stdio.h>\n");
		printf ("#include <string.h>\n");
		printf ("\n");
		printf ("struct kv { const char *name; const char *value; };\n");
		printf ("static const struct kv kvs[] = {\n");
	} else {
		printf ("%%{\n");
		printf ("// gperf -aclEDCIG --null-strings -H sdb_hash_c_%s -N sdb_get_c_%s -t %s.gperf > %s.c\n", cname, cname, cname, cname);
		printf ("// gcc -DMAIN=1 %s.c ; ./a.out > %s.h\n", cname, cname);
		printf ("#include <stdio.h>\n");
		printf ("#include <string.h>\n");
		printf ("#include <ctype.h>\n");
		printf ("%%}\n");
		printf ("\n");
		printf ("struct kv { const char *name; const char *value; };\n");
		printf ("%%%%\n");
	}
}

// TODO rename gperf with cgen
SDB_API void sdb_cgen_footer(const char *name, const char *cname, bool textmode) {
	if (textmode) {
		printf ("  {NULL, NULL}\n");
		printf ("};\n");
		printf ("// %p\n", cname);
		printf ("// TODO\n");
		printf ("typedef int (*GperfForeachCallback)(void *user, const char *k, const char *v);\n");
		printf ("int gperf_%s_foreach(GperfForeachCallback cb, void *user) {\n", cname);
		printf ("  int i = 0; while (kvs[i].name) {\n");
		printf ("  cb (user, kvs[i].name, kvs[i].value);\n");
		printf ("  i++;}\n");
		printf ("  return 0;\n");
		printf ("}\n");
		printf ("const char *gperf_%s_get(const char *s) {\n", cname);
		printf ("  int i = 0; while (kvs[i].name) {\n");
		printf ("  if (!strcmp (s, kvs[i].name)) return kvs[i].value;\n");
		printf ("  i++;}\n");
		printf ("  return NULL;\n");
		printf ("}\n");
		printf ("#define sdb_hash_c_%s(x,y) gperf_%s_hash(x)\n", cname, cname);
		printf ("const unsigned int gperf_%s_hash(const char *s) {\n", cname);
		printf ("  int sum = strlen (s);\n");
		printf ("  while (*s) { sum += *s; s++; }\n");
		printf ("  return sum;\n");
		printf ("}\n");
		printf (
			"struct {const char *name;void *get;void *hash;void *foreach;} gperf_%s = {\n"
			"  .name = \"%s\",\n"
			"  .get = &gperf_%s_get,\n"
			"  .hash = &gperf_%s_hash,\n"
			"  .foreach = &gperf_%s_foreach\n"
			"};\n", cname, name, cname, cname, cname);
		printf (
			"\n"
			"#if MAIN\n"
			"int main () {\n"
			"	const char *s = ((char*(*)(char*))gperf_%s.get)(\"foo\");\n"
			"	printf (\"%%s\\n\", s);\n"
			"}\n"
			"#endif\n", cname);
		return;
	}
	printf ("%%%%\n");
	printf ("// SDB-CGEN V" SDB_VERSION "\n");
	printf ("// %p\n", cname);
	printf ("typedef int (*GperfForeachCallback)(void *user, const char *k, const char *v);\n");
	printf ("int gperf_%s_foreach(GperfForeachCallback cb, void *user) {\n", cname);
	printf ("\tint i;for (i=0;i<TOTAL_KEYWORDS;i++) {\n");
	printf ("\tconst struct kv *w = &wordlist[i];\n");
	printf ("\tif (!cb (user, w->name, w->value)) return 0;\n");
	printf ("}\n");
	printf ("return 1;}\n");
	printf ("const char* gperf_%s_get(const char *s) {\n", cname);
	printf ("\tconst struct kv *o = sdb_get_c_%s (s, strlen(s));\n", cname);
	printf ("\treturn o? o->value: NULL;\n");
	printf ("}\n");
	printf ("const unsigned int gperf_%s_hash(const char *s) {\n", cname);
	printf ("\treturn sdb_hash_c_%s(s, strlen (s));\n", cname);
	printf ("}\n");
	printf (
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
		"		if (*line == '%%' && line[1] == '%%')\n"
		"			mode++;\n"
		"	}\n"
		"	printf (\"#endif\\n\");\n"
		"}\n"
		"#endif\n",
		cname, cname, cname, cname, cname,
		name, name,
		cname, cname, cname, cname
	);
	printf ("\n");
}

