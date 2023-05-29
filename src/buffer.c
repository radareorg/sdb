/* Public Domain */

#include "sdb/buffer.h"

void buffer_init(buffer *s, BufferOp op, int fd, char *buf, size_t len) {
	s->x = buf;
	s->fd = fd;
	s->op = op;
	s->p = 0;
	s->n = len;
}

static int allwrite(BufferOp op, int fd, const char *buf, size_t len) {
	while (len > 0) {
		size_t w = op (fd, buf, len);
		if (w != len) {
			return 0;
		}
		buf += w;
		len -= w;
	}
	return 1;
}

int buffer_flush(buffer *s) {
	size_t p = s->p;
	if (!p) {
		return 1;
	}
	s->p = 0;
	return allwrite (s->op, s->fd, s->x, p);
}

int buffer_putalign(buffer *s, const char *buf, size_t len) {
	ut32 n;
	if (!s || !s->x || !buf) {
		return 0;
	}
	while (len > (n = s->n - s->p)) {
		memcpy (s->x + s->p, buf, n);
		s->p += n; buf += n; len -= n;
		if (!buffer_flush (s)) {
			return 0;
		}
	}
	/* now len <= s->n - s->p */
	memcpy (s->x + s->p, buf, len);
	s->p += len;
	return 1;
}

int buffer_putflush(buffer *s, const char *buf, size_t len) {
	if (!buffer_flush (s)) {
		return 0;
	}
	return allwrite (s->op, s->fd, buf, len);
}
