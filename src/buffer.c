#include "buffer.h"

void buffer_init(buffer *s,BufferOp op,int fd,char *buf,unsigned int len) {
	s->x = buf;
	s->fd = fd;
	s->op = op;
	s->p = 0;
	s->n = len;
}

static int allwrite(BufferOp op,int fd,const char *buf,unsigned int len) {
	int w;
	while (len) {
		w = op(fd,buf,len);
		if (w == -1)
			return -1; /* note that some data may have been written */
		//if (w == 0) ; /* luser's fault WAT! */
		buf += w;
		len -= w;
	}
	return 0;
}

int buffer_flush(buffer *s) {
	int p = s->p;
	if (!p) return 0;
	s->p = 0;
	return allwrite (s->op, s->fd, s->x, p);
}

int buffer_putalign(buffer *s,const char *buf,unsigned int len) {
	unsigned int n;

	while (len > (n = s->n - s->p)) {
		byte_copy(s->x + s->p,n,buf); s->p += n; buf += n; len -= n;
		if (buffer_flush(s) == -1) return -1;
	}
	/* now len <= s->n - s->p */
	byte_copy(s->x + s->p,len,buf);
	s->p += len;
	return 0;
}

#if 0
int buffer_put(buffer *s,const char *buf,unsigned int len) {
	unsigned int n = s->n;
	if (len > n - s->p) {
		if (buffer_flush(s) == -1) return -1;
		/* now s->p == 0 */
		if (n < BUFFER_OUTSIZE) n = BUFFER_OUTSIZE;
		while (len > s->n) {
			if (n > len) n = len;
			if (allwrite (s->op,s->fd,buf,n) == -1) return -1;
			buf += n;
			len -= n;
		}
	}
	/* now len <= s->n - s->p */
	byte_copy(s->x + s->p,len,buf);
	s->p += len;
	return 0;
}
#endif

int buffer_putflush(buffer *s,const char *buf,unsigned int len) {
	if (buffer_flush(s) == -1) return -1;
	return allwrite (s->op,s->fd,buf,len);
}
