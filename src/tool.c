#include <sdb/sdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(_WIN32)
#include <direct.h> /* for _getcwd, _chdir */
#include <io.h>     /* for _unlink */
#define getcwd _getcwd
#define chdir _chdir
#define unlink _unlink
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#if HAVE_GPERF
// #define COMPILE_GPERF 1
#define COMPILE_GPERF 0
#else
#define COMPILE_GPERF 0
#endif

static char *get_name(const char *name) {
	const char *l = name + strlen(name) - 1;
	while (*l && l > name) {
		if (*l == '/') {
			name = l + 1;
			break;
		}
		l--;
	}
	char *n = sdb_strdup(name);
	char *v, *d = n;
	for (v = n; *v; v++) {
		if (*v == '.') {
			break;
		}
		*d++ = *v;
	}
	*d++ = 0;
	return n;
}

static char *get_cname(const char *name) {
	const char *l = name + strlen(name) - 1;
	while (*l && l > name) {
		if (*l == '/') {
			name = l + 1;
			break;
		}
		l--;
	}
	char *n = sdb_strdup(name);
	char *v, *d = n;
	for (v = n; *v; v++) {
		if (*v == '/' || *v == '-') {
			*d++ = '_';
			continue;
		}
		if (*v == '.') {
			break;
		}
		*d++ = *v;
	}
	*d++ = 0;
	return n;
}

static char *escape(const char *b, int ch) {
	char *a = (char *)sdb_gh_calloc((1 + strlen(b)), 4);
	if (!a) {
		return NULL;
	}
	char *c = a;
	while (*b) {
		if (*b == ch) {
			*c = '_';
		} else switch (*b) {
		case '"':
			*c++ = '\\';
			*c++ = '"';
			break;
		case '\\':
			*c++ = '\\';
			*c++ = '\\';
			break;
		case '\r':
			*c++ = '\\';
			*c++ = 'r';
			break;
		case '\n':
			*c++ = '\\';
			*c++ = 'n';
			break;
		case '\t':
			*c++ = '\\';
			*c++ = 't';
			break;
		default:
			*c = *b;
			break;
		}
		b++;
		c++;
	}
	return a;
}

static bool dothec(const char *file_txt, const char *file_gperf, const char *file_c, bool compile_gperf) {
	Sdb *db = NULL;
	char *header = NULL;
	char *footer = NULL;
	char *content = NULL;
	StrBuf *sb = NULL;
	// Variables declared at top to avoid jumping over initializations in C++
	SdbList *l;
	SdbKv *kv;
	SdbListIter *it;
	char *ek;
	char *ev;
	FILE *f;
	size_t content_len;
	size_t written;

	// Extract name and cname from file_txt
	char *name = get_name (file_txt);
	char *cname = get_cname (file_txt);

	bool textmode = !compile_gperf;
	// Generate header string using the sdb API
	header = sdb_cgen_header (cname, textmode);
	if (!header) {
		fprintf (stderr, "Failed to generate header\n");
		sdb_gh_free (name);
		sdb_gh_free (cname);
		return false;
	}
	// Create a string buffer for the entire file content
	sb = strbuf_new ();
	if (!sb) {
		fprintf (stderr, "Failed to create string buffer\n");
		goto fail;
	}
	// Add header to the buffer
	strbuf_append (sb, header, 0);
	sdb_gh_free (header);
	// Read key-value pairs from the SDB file
	db = sdb_new (NULL, NULL, 0);
	if (!db) {
		fprintf (stderr, "Failed to create SDB instance\n");
		goto fail;
	}
	if (!sdb_text_load (db, file_txt)) {
		fprintf (stderr, "Failed to load SDB text file %s\n", file_txt);
		goto fail;
	}
	// Iterate and collect all key-value pairs in the string buffer
   l = sdb_foreach_list (db, true);
	ls_foreach_cast (l, it, SdbKv*, kv) {
		const char *k = sdbkv_key (kv);
		const char *v = sdbkv_value (kv);

		// Escape special characters
		ek = escape (k, ',');
		ev = escape (v, 0);

		if (ek && ev) {
			strbuf_appendf (sb, 0, "\t{\"%s\", \"%s\"},\n", ek, ev);
			sdb_gh_free (ek);
			sdb_gh_free (ev);
		}
	}
	ls_free (l);

	// Generate footer string using the sdb API
	footer = sdb_cgen_footer (name, cname, textmode);
	if (!footer) {
		fprintf (stderr, "Failed to generate footer\n");
		goto fail;
	}
	// Add footer to the buffer
	strbuf_append (sb, footer, 0);
	sdb_gh_free (footer);

	// Get the complete file content and write it to the file
	content = strbuf_drain (sb);
	if (!content) {
		fprintf (stderr, "Failed to create file content\n");
		goto fail;
	}

	// Write the complete content to file
   f = fopen(file_gperf, "wb");
	if (!f) {
		fprintf (stderr, "Failed to open file %s for writing\n", file_gperf);
		sdb_gh_free (content);
		goto fail;
	}
   content_len = strlen(content);
   written = fwrite(content, 1, content_len, f);
	fclose(f);
	if (written != content_len) {
		fprintf (stderr, "Failed to write to file %s\n", file_gperf);
		sdb_gh_free (content);
		goto fail;
	}
	sdb_gh_free (content);

	fprintf (stdout, "Generated GPERF file: %s\n", file_gperf);
	if (compile_gperf) {
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "gperf -aclEDCIG --null-strings -H sdb_hash_c_%s"
				" -N sdb_get_c_%s -t %s > %s", cname, cname, file_gperf, file_c);
		fprintf (stdout, "Generating C file from gperf: %s\n", file_c);
		fprintf (stderr, "system: %s\n", cmd);
		int rc = system(cmd);
		if (rc == 0) {
			fprintf (stdout, "Done\n");
			unlink (file_gperf);
		} else {
			fprintf (stderr, "Cannot generate C file: %s\n", file_c);
		}
	}
	sdb_free (db);
	sdb_gh_free (name);
	sdb_gh_free (cname);
	return true;
fail:
	strbuf_free (sb);
	sdb_gh_free (name);
	sdb_gh_free (cname);
	sdb_gh_free (content);
	sdb_gh_free (header);
	sdb_free (db);
	return false;
}

static bool dothesdb(const char *file_txt, const char *file_sdb) {
	Sdb *db = sdb_new (NULL, file_sdb, 0);
	if (sdb_text_load (db, file_txt)) {
		fprintf (stderr, "maked %s\n", file_sdb);
		sdb_sync (db);
		sdb_free (db);
		return true;
	}
	fprintf (stderr, "Failed to parse %s\n", file_txt);
	sdb_free (db);
	return false;
}

static bool file_exists(const char *path) {
	struct stat st;
	return stat(path, &st) == 0;
}

static bool is_newer(const char *path_a, const char *path_b) {
	struct stat sta, stb;
	if (stat(path_a, &sta) != 0) {
		return false;
	}
	if (stat(path_b, &stb) != 0) {
		return true;
	}
	return sta.st_mtime > stb.st_mtime;
}

static bool dothething(const char *basedir, const char *file_txt) {
	bool compile_gperf = COMPILE_GPERF;
	char *file_sdb = sdb_strdup (file_txt);
	if (!file_sdb) {
		return false;
	}
	file_sdb [strlen (file_txt) - 4] = 0;
	
	char *file_c = sdb_strdup (file_sdb);
	if (!file_c) {
		sdb_gh_free (file_sdb);
		return false;
	}
	strcpy (file_c + strlen (file_c) - 3, "c");
	
	char *file_gperf = sdb_strdup(file_c);
	if (!file_gperf) {
		sdb_gh_free(file_c);
		sdb_gh_free(file_sdb);
		return false;
	}

	const char *file_ref = compile_gperf? file_c: file_gperf;
	if (!file_exists(file_ref) || is_newer(file_txt, file_ref)) {
		fprintf (stdout, "newer %s\n", file_c);
		dothec(file_txt, file_gperf, file_c, compile_gperf);
	}
	if (!file_exists(file_sdb) || is_newer(file_txt, file_sdb)) {
		fprintf (stdout, "newer %s\n", file_sdb);
		dothesdb(file_txt, file_sdb);
	}
	sdb_gh_free(file_c);
	sdb_gh_free(file_gperf);
	sdb_gh_free(file_sdb);
	return true;
}
SDB_API bool sdb_tool(const char *path) {
	if (!path) {
		fprintf(stderr, "Usage: sdb -r [path]\n");
		return false;
	}

	DIR *dir = opendir(path);
	if (!dir) {
		fprintf(stderr, "Invalid directory: %s\n", path);
		return false;
	}

	// Change to the specified directory
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		fprintf(stderr, "Failed to get current directory\n");
		closedir(dir);
		return false;
	}

	if (chdir(path) != 0) {
		fprintf(stderr, "Cannot chdir to %s\n", path);
		closedir(dir);
		return false;
	}

	struct dirent *entry;
	bool success = false;
	while ((entry = readdir(dir)) != NULL) {
		const char *file = entry->d_name;
		size_t file_len = strlen(file);

		// Check if file ends with ".sdb.txt"
		if (file_len > 8 && strcmp (file + file_len - 8, ".sdb.txt") == 0) {
			success |= dothething (path, file);
		}
	}

	// Return to original directory
	if (chdir(cwd) != 0) {
		fprintf(stderr, "Warning: Failed to return to original directory\n");
	}

	closedir(dir);
	return success;
}
