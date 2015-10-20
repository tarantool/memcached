#ifndef TARANTOOL_MEMCACHED_NETWORK_H_INCLUDED
#define TARANTOOL_MEMCACHED_NETWORK_H_INCLUDED

#define TIMEOUT_INFINITY 365*86400*100.0

#include <small/ibuf.h>

ssize_t
mnet_writev(int fd, struct iovec *iov, int iovcnt, size_t size_hint);

ssize_t
mnet_write(int fd, void *buf, size_t sz);

ssize_t
mnet_read_ahead(int fd, void *buf, size_t bufsz, size_t sz);

static inline ssize_t
mnet_read_ibuf(int fd, struct ibuf *buf, size_t sz)
{
	ibuf_reserve(buf, sz);
	ssize_t n = mnet_read_ahead(fd, buf->wpos, ibuf_unused(buf), sz);
	buf->wpos += n;
	return n;
}

static inline int
mnet_move_iov(struct iovec *iov, ssize_t nwr, size_t *iov_len)
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

#endif /* TARANTOOL_MEMCACHED_NETWORK_H_INCLUDED */
