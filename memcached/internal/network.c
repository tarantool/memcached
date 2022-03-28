#include <errno.h>

#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#include <tarantool/module.h>
#include <small/mempool.h>
#include <small/obuf.h>

#include "alloc.h"
#include "memcached.h"
#include "memcached_ibuf.h"
#include "constants.h"
#include "network.h"

#ifndef IOV_MAX
#define IOV_MAX UIO_MAXIOV
#endif

static __thread struct mempool ibuf_pool, obuf_pool;

static int iobuf_readahead = 16320;

void
iobuf_mempool_create()
{
	static __thread bool inited = false;
	if (inited) return;
	mempool_create(&ibuf_pool, memcached_slab_cache(), sizeof(struct ibuf));
	mempool_create(&obuf_pool, memcached_slab_cache(), sizeof(struct obuf));
	inited = 1;
}

void
iobuf_mempool_destroy()
{
	mempool_destroy(&ibuf_pool);
	mempool_destroy(&obuf_pool);
}

struct ibuf *
ibuf_new()
{
	void *ibuf = mempool_alloc(&ibuf_pool);
	if (ibuf == NULL) return NULL;
	memcached_ibuf_create((struct ibuf *)ibuf, memcached_slab_cache(),
			      iobuf_readahead);
	return ibuf;
}

struct obuf *
obuf_new()
{
	void *obuf = mempool_alloc(&obuf_pool);
	if (obuf == NULL) return NULL;
	obuf_create((struct obuf *)obuf, memcached_slab_cache(), iobuf_readahead);
	return obuf;
}

void
iobuf_delete(struct ibuf *ibuf, struct obuf *obuf)
{
	memcached_ibuf_destroy(ibuf);
	obuf_destroy(obuf);
	mempool_free(&ibuf_pool, ibuf);
	mempool_free(&obuf_pool, obuf);
}

size_t
mnet_writev(int fd, struct iovec *iov, int iovcnt, size_t size_hint)
{
	size_t iov_len = 0;
	struct iovec *end = iov + iovcnt;

	size_t written = 0;
	if (size_hint == 0) return 0;
	while (true) {
		int cnt = iovcnt < IOV_MAX ? iovcnt : IOV_MAX;
		ssize_t n = writev(fd, iov, cnt);
		if (n < 0 && errno != EAGAIN &&
			     errno != EWOULDBLOCK &&
			     errno != EINTR) {
			return written;
		} else if (n > 0) {
			written += n;
			if (size_hint > 0 && size_hint <= written) {
				return written;
			}
			iov += mnet_move_iov(iov, n, &iov_len);
			if (iov == end) {
				assert(iov_len == 0);
				return written;
			}
		}
		coio_wait(fd, COIO_WRITE, TIMEOUT_INFINITY);
	}
}

size_t
mnet_write(int fd, void *buf, size_t sz)
{
	size_t written = 0;
	while (true) {
		ssize_t nwr = write(fd, buf, sz - written);
		if (nwr > 0) {
			written += nwr;
			if (written >= sz)
				return sz;
			buf = (char *)buf + nwr;
		} else if (nwr <= 0 && errno != EAGAIN &&
				       errno != EWOULDBLOCK &&
				       errno != EINTR) {
			return written;
		}
		coio_wait(fd, COIO_WRITE, TIMEOUT_INFINITY);
	}
}

size_t
mnet_read_ahead(int fd, void *buf, size_t bufsz, size_t sz)
{
	size_t total = 0;
	while (true) {
		ssize_t nrd = read(fd, buf, bufsz);
		if (nrd > 0) {
			total += nrd;
			if (total >= sz)
				return total;
			buf = (char *) buf + nrd;
			bufsz -= nrd;
		} else if (nrd == 0 || errno == ECONNRESET) {
			errno = 0;
			return total;
		} else if (errno != EWOULDBLOCK &&
			   errno != EAGAIN &&
			   errno != EINTR) {
			return total;
		}
		coio_wait(fd, COIO_READ, TIMEOUT_INFINITY);
	}
}

size_t
mnet_read_ibuf(int fd, struct ibuf *buf, size_t sz)
{
	if (memcached_ibuf_reserve(buf, sz) == NULL) {
		return -1;
	}
	ssize_t n = mnet_read_ahead(fd, buf->wpos, memcached_ibuf_unused(buf),
				    sz);
	buf->wpos += n;
	return n;
}

int
mnet_setsockopt_keepalive(int fd) {
	/* k is for keep */
	int on = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) == -1) {
		return -1;
	}

#ifdef    __linux__
	int kcnt = 5, kidle = 30, kintvl = 60;

	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT,   &kcnt,   sizeof(int)) == -1 ||
	    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE,  &kidle,  sizeof(int)) == -1 ||
	    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &kintvl, sizeof(int)) == -1) {
		return -1;
	}
#endif /* __linux__ */

	return 0;
}

int
mnet_setsockopt(int fd, const char *family, const char *type) {
	int on = 1;
	struct linger linger = {0, 0};

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1 ||
	    setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		return -1;
	}
	if (strcmp(type, "SOCK_STREAM") == 0 && strcmp(family, "AF_UNIX") != 0) {
		if (mnet_setsockopt_keepalive(fd) == -1) {
			return -1;
		}
	}

	return 0;
}
