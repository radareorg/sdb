int js0n(unsigned char *js, unsigned int len, unsigned short *out);
Rangstr json_get (const char *s, const char *path);

/* SDB public api */
// XXX: 1st arg should be Sdb*
char *sdb_json_get (const char *s, const char *p);
char *sdb_json_set (const char *s, const char *k, const char *v);
int sdb_json_geti (const char *s, const char *p);
char *sdb_json_seti (const char *s, const char *k, int a);
char *sdb_json_unindent(const char *s);
char *sdb_json_indent(const char *s);
