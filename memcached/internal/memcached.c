/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>

#include <tarantool/module.h>
#include <small/ibuf.h>
#include <small/obuf.h>

#include "memcached.h"
#include "memcached_layer.h"
#include "error.h"
#include "network.h"
#include "proto_binary.h"
#include "proto_text.h"
#include "expiration.h"

static inline int
memcached_skip_request(struct memcached_connection *con) {
	struct ibuf *in = con->in;
	while (ibuf_used(in) < con->len && con->noprocess) {
		con->len -= ibuf_used(in);
		ibuf_reset(in);
		ssize_t read = mnet_read_ibuf(con->fd, in, 1);
		if (read == -1)
			memcached_error_ENOMEM(1, "ibuf");
		if (read < 1) {
			return -1;
		}
		con->cfg->stat.bytes_read += read;
	}
	in->rpos += con->len;
	return 0;
}

static inline ssize_t
memcached_flush(struct memcached_connection *con) {
	ssize_t total = mnet_writev(con->fd, con->out->iov,
				    obuf_iovcnt(con->out),
				    obuf_size(con->out));
	con->cfg->stat.bytes_written += total;
	if (ibuf_used(con->in) == 0)
		ibuf_reset(con->in);
	obuf_reset(con->out);
	if (ibuf_reserve(con->in, con->cfg->readahead) == NULL)
		return -1;
	return total;
}

static inline int
memcached_loop_read(struct memcached_connection *con, size_t to_read)
{
	if (ibuf_reserve(con->in, to_read) == NULL) {
/*		memcached_error_ENOMEM(to_read, "ibuf");*/
		return -1;
	}
	ssize_t read = mnet_read_ibuf(con->fd, con->in, to_read);
	if (read == -1)
		memcached_error_ENOMEM(to_read, "ibuf");
	if (read < (ssize_t )to_read) {
		return -1;
	}
	con->cfg->stat.bytes_read += read;
	return 0;
}

static inline int
memcached_loop_error(struct memcached_connection *con) {
	int errcode = 0;
	const char *errstr = NULL;
	if (con->errcode > 0) {
		box_error_t *error = box_error_last();
		if (!error) return 0;
		errcode = box_error_code(error);
		errstr = box_error_message(error);
		con->errcode = 0;
	} else {
		errcode = con->errcode;
		errstr = memcached_get_result_description(errcode);
	}
	if (errcode > box_error_code_MAX) {
		errcode -= box_error_code_MAX;
		/* TODO proper retval checking */
		con->cb.process_error(con, errcode, errstr);
	} else {
		/* TODO proper retval checking */
		memcached_error_SERVER_ERROR("SERVER ERROR %d: %s", errcode,
					     errstr);
	}
	box_error_clear();
	return 0;
}

static inline int
memcached_loop_negotiate(struct memcached_connection *con)
{
	const char symbol = *(con->in->rpos);
	if (symbol == (const char)MEMCACHED_BIN_REQUEST) {
		memcached_set_binary(con);
	} else {
		memcached_set_text(con);
	}
	return con->cb.parse_request(con);
}

static inline void
memcached_loop(struct memcached_connection *con)
{
	int rc = 0;
	size_t to_read = 1;
	int batch_count = 0;

	for (;;) {
		rc = memcached_loop_read(con, to_read);
		if (rc == -1) {
			/**
			 * We can't read input (OOM or SocketError)
			 * We're closing connection anyway and don't reply.
			 */
			break;
		}
		to_read = 1;
next:
		con->noreply = false;
		con->noprocess = false;
		rc = con->cb.parse_request(con);
		if (rc == -1) {
			memcached_loop_error(con);
			con->write_end = obuf_create_svp(con->out);
			memcached_skip_request(con);
			if (con->close_connection) {
				/* If magic is wrong we'll close connection */
				say_info("Bad magic or exit. Exiting.");
				break;
			}
			memcached_flush(con);
			batch_count = 0;
			continue;
		} else if (rc > 0) {
			to_read = rc;
			batch_count = 0;
			continue;
		}
		assert(!con->close_connection);
		rc = 0;
		if (!con->noprocess) rc = con->cb.process_request(con);
		con->write_end = obuf_create_svp(con->out);
		memcached_skip_request(con);
		if (rc == -1)
			memcached_loop_error(con);
		if (con->close_connection) {
			say_debug("Requesting exit. Exiting.");
			break;
		} else if (rc == 0 && ibuf_used(con->in) > 0 &&
			   batch_count < con->cfg->batch_count) {
			batch_count++;
			goto next;
		}
		/* Write back answer */
		if (!con->noreply)
			memcached_flush(con);
		fiber_sleep(0);
		batch_count = 0;
		continue;
	}
	memcached_flush(con);
}

void
memcached_handler(struct memcached_service *p, int fd)
{
	struct memcached_connection con;
	/* TODO: move to connection_init */
	memset(&con, 0, sizeof(struct memcached_connection));
	con.fd        = fd;
	con.in        = ibuf_new();
	con.out       = obuf_new();
	con.write_end = obuf_create_svp(con.out);
	con.cfg       = p;

	/* prepare connection type */
	if (p->proto == MEMCACHED_PROTO_NEGOTIATION) {
		con.cb.parse_request = memcached_loop_negotiate;
	} else if (p->proto == MEMCACHED_PROTO_BINARY) {
		memcached_set_binary(&con);
	} else if (p->proto == MEMCACHED_PROTO_TEXT) {
		memcached_set_text(&con);
	} else {
		assert(0); /* unreacheable */
	}

	/* read-write cycle */
	con.cfg->stat.curr_conns++;
	con.cfg->stat.total_conns++;
	memcached_loop(&con);
	/* close connection and reflect it in stats */
	con.cfg->stat.curr_conns--;
	close(con.fd);
	iobuf_delete(con.in, con.out);
	const box_error_t *err = box_error_last();
	if (err) say_error("%s", box_error_message(err));
}

struct memcached_service*
memcached_create(const char *name, uint32_t sid)
{
	iobuf_mempool_create();
	struct memcached_service *srv = (struct memcached_service *)
		calloc(1, sizeof(struct memcached_service));
	if (!srv) {
		say_syserror("failed to allocate memory for memcached service");
		return NULL;
	}
	srv->batch_count    = 20;
	srv->expire_enabled = true;
	srv->expire_count   = 50;
	srv->expire_time    = 3600;
	srv->expire_fiber   = NULL;
	srv->space_id       = sid;
	srv->name           = strdup(name);
	srv->cas            = 1;
	srv->readahead      = 16384;
	if (!srv->name) {
		say_syserror("failed to allocate memory for memcached service");
		free(srv);
		return NULL;
	}
	return srv;
}

void
memcached_free(struct memcached_service *srv)
{
	memcached_stop(srv);
	if (srv) free((void *)srv->name);
	free(srv);
}

int
memcached_start (struct memcached_service *srv)
{
	if (memcached_expire_start(srv) == -1)
		return -1;
	return 0;
}

void
memcached_stop (struct memcached_service *srv)
{
	memcached_expire_stop(srv);
	while (srv->stat.curr_conns != 0)
		fiber_sleep(0.001);
}

void
memcached_set_opt (struct memcached_service *srv, int opt, ...)
{
	va_list va; va_start(va, opt);
	switch (opt) {
	case MEMCACHED_OPT_READAHEAD:
		srv->readahead = (int )va_arg(va, double);
		break;
	case MEMCACHED_OPT_EXPIRE_ENABLED: {
		int flag = (int )va_arg(va, int);
		if (flag == 0) {
			srv->expire_enabled = false;
			memcached_expire_stop(srv);
		} else {
			srv->expire_enabled = true;
		}
		break;
	}
	case MEMCACHED_OPT_EXPIRE_COUNT:
		srv->expire_count = (uint32_t )va_arg(va, double);
		break;
	case MEMCACHED_OPT_EXPIRE_TIME:
		srv->expire_time = (uint32_t )va_arg(va, double);
		break;
	case MEMCACHED_OPT_FLUSH_ENABLED: {
		int flag = (int )va_arg(va, int);
		if (flag == 0) {
			srv->flush_enabled = false;
		} else {
			srv->flush_enabled = true;
		}
		break;
	}
	case MEMCACHED_OPT_VERBOSITY: {
		int flag = (int )va_arg(va, double);
		if (flag > 0) {
			srv->verbosity = (flag < 4 ? flag : 3);
		} else if (flag > 3) {
			srv->verbosity = 0;
		}
		break;
	}
	case MEMCACHED_OPT_PROTOCOL: {
		const char *type = va_arg(va, const char *);
		if (strncmp(type, "bin", 3) == 0) {
			srv->proto = MEMCACHED_PROTO_BINARY;
		} else if (strncmp(type, "negot", 5) == 0) {
			srv->proto = MEMCACHED_PROTO_NEGOTIATION;
		} else {
			srv->proto = MEMCACHED_PROTO_TEXT;
		}
		break;
	}
	default:
		say_error("No such option %d", opt);
		break;
	}
	va_end(va);
}

struct memcached_stat *memcached_get_stat (struct memcached_service *srv)
{
	return &(srv->stat);
}
