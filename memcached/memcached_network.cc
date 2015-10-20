#include <errno.h>

#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <coio.h>
#include <box/memcached_network.h>

ssize_t
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

ssize_t
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

ssize_t
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
