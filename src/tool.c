#include <sdb/sdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#if defined(_WIN32)
#include <direct.h> /* for _getcwd, _chdir */
#include <io.h>     /* for _unlink */
#include <windows.h>
#define getcwd _getcwd
#define chdir _chdir
#define unlink _unlink
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

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

static bool dothec(const char *file_txt, const char *file_gperf, const char *file_c, bool compile_gperf, bool mirror_mode, const char *output_dir) {
	// Create output directory if it doesn't exist
	if (output_dir) {
		#if defined(_WIN32)
		if (_mkdir(output_dir) != 0 && errno != EEXIST) {
			// Ignore if directory already exists
			fprintf(stderr, "Failed to create output directory: %s\n", output_dir);
		}
		#else
		if (mkdir(output_dir, 0755) != 0 && errno != EEXIST) {
			// Ignore if directory already exists
			fprintf(stderr, "Failed to create output directory: %s\n", output_dir);
		}
		#endif
	}
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
		ek = escape (k, 0); // ',');
		ev = escape (v, 0);

		if (ek && ev) {
			strbuf_appendf (sb, 0, "\t{\"%s\", \"%s\"},\n", ek, ev);
			if (mirror_mode) {
				char *comma = strchr (ev, ',');
				if (comma) {
					*comma++ = '.';
					comma = strchr (ev, ',');
					if (comma) {
						*comma++ = 0;
					}
				}
				if (strcmp (ek, "_")) {
					strbuf_appendf (sb, 0, "\t{\"%s\", \"%s\"},\n", ev, ek);
				}
			}
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

	fprintf (stdout, "SDBTOOL gperf=%s\n", file_gperf);
	if (compile_gperf) {
		char cmd[1024];
		snprintf (cmd, sizeof (cmd), "gperf -aclEDCIG --null-strings -H sdb_hash_c_%s"
				" -N sdb_get_c_%s -t %s > %s", cname, cname, file_gperf, file_c);
		fprintf (stdout, "Generating C file from gperf: %s\n", file_c);
		fprintf (stderr, "system: %s\n", cmd);
#if HAVE_SYSTEM
		int rc = system (cmd);
		if (rc == 0) {
			fprintf (stdout, "Done\n");
			unlink (file_gperf);
		} else {
			fprintf (stderr, "Cannot generate C file: %s\n", file_c);
		}
#else
		fprintf (stderr, "This build doesnt support running system commands.\n");
		fprintf (stderr, "%s\n", cmd);
#endif
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

static void mirror_sdb(Sdb *db) {
	SdbList *l = sdb_foreach_list (db, true);
	SdbListIter *it;
	SdbKv *kv;
	ls_foreach_cast (l, it, SdbKv*, kv) {
		const char *k = sdbkv_key (kv);
		const char *v = sdbkv_value (kv);

		// Escape special characters
		char *ek = escape (k, 0); // ',');
		char *ev = escape (v, 0);

		if (ek && ev) {
			char *comma = strchr (ev, ',');
			if (comma) {
				*comma++ = '.';
				comma = strchr (ev, ',');
				if (comma) {
					*comma++ = 0;
				}
			}
			if (strcmp (ek, "_")) {
				sdb_set (db, ev, ek, 0);
			}
		}
		sdb_gh_free (ek);
		sdb_gh_free (ev);
	}
}

static bool dothesdb(const char *file_txt, const char *file_sdb, bool mirror_mode, const char *output_dir) {
	// Create output directory if it doesn't exist
	if (output_dir) {
		#if defined(_WIN32)
		if (_mkdir(output_dir) != 0 && errno != EEXIST) {
			// Ignore if directory already exists
			fprintf(stderr, "Failed to create output directory: %s\n", output_dir);
		}
		#else
		if (mkdir(output_dir, 0755) != 0 && errno != EEXIST) {
			// Ignore if directory already exists
			fprintf(stderr, "Failed to create output directory: %s\n", output_dir);
		}
		#endif
	}
	Sdb *db = sdb_new (NULL, file_sdb, 0);
	if (sdb_text_load (db, file_txt)) {
		fprintf (stderr, "maked %s\n", file_sdb);
		if (mirror_mode) {
			mirror_sdb (db);
		}
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

static bool dothething(const char *basedir, const char *file_txt, bool mirror_mode, const char *output_dir) {
	bool compile_gperf = COMPILE_GPERF;
	char *file_sdb = sdb_strdup (file_txt);
	if (!file_sdb) {
		return false;
	}
	file_sdb [strlen (file_txt) - 4] = 0;
	// If output directory is specified, prepend it to output filenames
	char *out_file_sdb = NULL;
	if (output_dir) {
		size_t len = strlen(output_dir) + strlen(file_sdb) + 2; // +2 for '/' and null terminator
		out_file_sdb = (char *)sdb_gh_malloc(len);
		if (out_file_sdb) {
			snprintf(out_file_sdb, len, "%s/%s", output_dir, file_sdb);
			// Use the path with output directory for output files
			sdb_gh_free(file_sdb);
			file_sdb = out_file_sdb;
		}
	}

	char *file_c = NULL;
	if (output_dir) {
		// Extract the base filename without path for C file name
		const char *base_name = file_sdb;
		const char *slash = strrchr(file_sdb, '/');
		if (slash) {
			base_name = slash + 1;
		}
		size_t len = strlen(output_dir) + strlen(base_name) + 2;
		file_c = (char *)sdb_gh_malloc(len);
		if (!file_c) {
			sdb_gh_free(file_sdb);
			return false;
		}
		snprintf(file_c, len, "%s/%s", output_dir, base_name);
		strcpy (file_c + strlen (file_c) - 3, "c");
	} else {
		file_c = sdb_strdup (file_sdb);
		if (!file_c) {
			sdb_gh_free (file_sdb);
			return false;
		}
		strcpy (file_c + strlen (file_c) - 3, "c");
	}

	char *file_gperf = sdb_strdup(file_c);
	if (!file_gperf) {
		sdb_gh_free(file_c);
		sdb_gh_free(file_sdb);
		return false;
	}
#if 0
	// Add .gperf extension
	size_t gperf_len = strlen(file_gperf) + 7; // + .gperf + null terminator
	char *gperf_with_ext = (char *)sdb_gh_malloc(gperf_len);
	if (gperf_with_ext) {
		snprintf(gperf_with_ext, gperf_len, "%s.gperf", file_gperf);
		sdb_gh_free(file_gperf);
		file_gperf = gperf_with_ext;
	}
#endif

	const char *file_ref = compile_gperf? file_c: file_gperf;
	if (!file_exists(file_ref) || is_newer(file_txt, file_ref)) {
		fprintf (stdout, "newer %s\n", file_c);
		dothec (file_txt, file_gperf, file_c, compile_gperf, mirror_mode, output_dir);
	}
	if (!file_exists(file_sdb) || is_newer(file_txt, file_sdb)) {
		fprintf (stdout, "newer %s\n", file_sdb);
		dothesdb (file_txt, file_sdb, mirror_mode, output_dir);
	}
	sdb_gh_free(file_c);
	sdb_gh_free(file_gperf);
	sdb_gh_free(file_sdb);
	return true;
}

SDB_API bool sdb_tool(const char *path, bool mirror_mode) {
	if (!path) {
		fprintf(stderr, "Usage: sdb -r [path]\n");
		return false;
	}
	// Check for output directory environment variable
	const char *output_dir = getenv("SDB_OUTPUT_DIR");
	fprintf (stderr, "SDBTOOL (mirror=%d) from=%s to=%s\n", mirror_mode, path, output_dir? output_dir: path);
	bool success = false;
	bool nothing = true;

#if defined(_WIN32)
	/* Windows implementation using FindFirstFile */
	char cwd[1024];
	if (getcwd (cwd, sizeof (cwd)) == NULL) {
		fprintf(stderr, "Failed to get current directory\n");
		return false;
	}
	if (chdir(path) != 0) {
		fprintf(stderr, "Cannot chdir to %s\n", path);
		return false;
	}
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA ("*.sdb.txt", &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError ();
		if (err != ERROR_FILE_NOT_FOUND) {
			fprintf (stderr, "FindFirstFile failed for %s: error %lu\n", path, err);
			chdir (cwd);
			return false;
		}
	} else {
		do {
			const char *file = findData.cFileName;
			size_t len = strlen (file);
			if (len > 8 && strcmp (file + len - 8, ".sdb.txt") == 0) {
				// If in mirror mode, process the file contents with the mirror transformation
				success |= dothething(path, file, mirror_mode, output_dir);
			}
			nothing = false;
		} while (FindNextFileA (hFind, &findData));
		FindClose (hFind);
	}
	if (chdir (cwd) != 0) {
		fprintf(stderr, "Warning: Failed to return to original directory\n");
	}
#else
	DIR *dir = opendir (path);
	if (!dir) {
		fprintf (stderr, "Invalid directory: %s\n", path);
		return false;
	}

	// Change to the specified directory
	char cwd[1024];
	if (getcwd (cwd, sizeof (cwd)) == NULL) {
		fprintf (stderr, "Failed to get current directory\n");
		closedir (dir);
		return false;
	}

	if (chdir (path) != 0) {
		fprintf (stderr, "Cannot chdir to %s\n", path);
		closedir (dir);
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir (dir)) != NULL) {
		const char *file = entry->d_name;
		size_t file_len = strlen (file);
		nothing = false;

		// Check if file ends with ".sdb.txt"
		if (file_len > 8 && strcmp (file + file_len - 8, ".sdb.txt") == 0) {
			success |= dothething (path, file, mirror_mode, output_dir);
		}
	}

	// Return to original directory
	if (chdir (cwd) != 0) {
		fprintf(stderr, "Warning: Failed to return to original directory\n");
	}

	closedir (dir);
#endif
	const bool res = success || nothing;
	if (!res) {
		fprintf (stderr, "error: sdbtool failed\n");
	}
	return res;
}
