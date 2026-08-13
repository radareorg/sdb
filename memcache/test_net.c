/* mcsdb - LGPLv3 - Copyright 2026 - pancake */

#include "mcsdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PAYLOAD_SIZE (MCSDB_MAX_BUFFER * 2 + 17)

int main(void) {
	static const char prefix[] = "prefix:";
	static const char suffix[] = ":suffix";
	const size_t expected_size = sizeof (prefix) - 1 + PAYLOAD_SIZE + sizeof (suffix) - 1;
	char *payload = NULL;
	char *expected = NULL;
	char *actual = NULL;
	size_t actual_size = 0;
	FILE *file = NULL;
	int ret = 1;
	int fd;

	payload = malloc (PAYLOAD_SIZE + 1);
	expected = malloc (expected_size);
	actual = malloc (expected_size + 1);
	file = tmpfile ();
	if (!payload || !expected || !actual || !file) {
		fprintf (stderr, "Cannot allocate test resources\n");
		goto beach;
	}
	memset (payload, 'A', PAYLOAD_SIZE);
	payload[PAYLOAD_SIZE] = 0;
	memcpy (expected, prefix, sizeof (prefix) - 1);
	memcpy (expected + sizeof (prefix) - 1, payload, PAYLOAD_SIZE);
	memcpy (expected + sizeof (prefix) - 1 + PAYLOAD_SIZE,
		suffix, sizeof (suffix) - 1);

	fd = fileno (file);
	if (net_printf (fd, "%s", prefix) != sizeof (prefix) - 1
		|| net_printf (fd, "%s", payload) != PAYLOAD_SIZE
		|| net_printf (fd, "%s", suffix) != sizeof (suffix) - 1
		|| net_flush (fd) < 0) {
		fprintf (stderr, "Cannot write formatted output\n");
		goto beach;
	}
	if (lseek (fd, 0, SEEK_SET) < 0) {
		fprintf (stderr, "Cannot rewind test output\n");
		goto beach;
	}
	while (actual_size < expected_size) {
		ssize_t n = read (fd, actual + actual_size, expected_size - actual_size);
		if (n <= 0) {
			break;
		}
		actual_size += n;
	}
	if (actual_size != expected_size || memcmp (actual, expected, expected_size)) {
		fprintf (stderr, "Formatted output was truncated or corrupted\n");
		goto beach;
	}
	if (read (fd, actual + expected_size, 1) != 0) {
		fprintf (stderr, "Formatted output exceeded the expected length\n");
		goto beach;
	}
	ret = 0;

beach:
	if (file) {
		fclose (file);
	}
	free (actual);
	free (expected);
	free (payload);
	return ret;
}
