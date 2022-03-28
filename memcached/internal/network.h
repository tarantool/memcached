#ifndef   NETWORK_H_INCLUDED
#define   NETWORK_H_INCLUDED

#define TIMEOUT_INFINITY 365*86400*100.0

#include "memcached_ibuf.h"

size_t
mnet_writev(int fd, struct iovec *iov, int iovcnt, size_t size_hint);

size_t
mnet_write(int fd, void *buf, size_t sz);

size_t
mnet_read_ahead(int fd, void *buf, size_t bufsz, size_t sz);

size_t
mnet_read_ibuf(int fd, struct ibuf *buf, size_t sz);

static inline int
mnet_move_iov(struct iovec *iov, size_t nwr, size_t *iov_len)
{
	nwr += *iov_len;
	struct iovec *begin = iov;
	while (nwr > 0 && nwr >= iov->iov_len) {
		nwr -= iov->iov_len;
		iov++;
	}
	*iov_len = nwr;
	return iov - begin;
}

static inline void
mnet_add_to_iov(struct iovec *iov, ssize_t size)
{
	iov->iov_len += size;
	iov->iov_base = (char *) iov->iov_base - size;
}

void
iobuf_mempool_create();

void
iobuf_mempool_destroy();

struct ibuf *
ibuf_new();

struct obuf *
obuf_new();

void
iobuf_delete(struct ibuf *ibuf, struct obuf *obuf);

int
mnet_setsockopt(int fd, const char *family, const char *type);
#endif /* NETWORK_H_INCLUDED */
