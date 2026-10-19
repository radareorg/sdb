#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <stddef.h>

size_t json_escape_len(const char *s);
char *json_escape(const char *s);

#endif
