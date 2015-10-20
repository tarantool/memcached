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

#include "say.h"
#include "evio.h"
#include "main.h"
#include "fiber.h"
#include "iobuf.h"
#include "scoped_guard.h"

#include "box/box.h"
#include "box/txn.h"
#include "box/tuple.h"
#include "box/index.h"
//#include "box/session.h"

#include "lib/bit/bit.h"
#include "lib/salad/fifo.h"

#include "box/memcached.h"
#include "box/memcached_layer.h"

#include "box/memcached_network.h"

static inline int
memcached_need_txn(struct memcached_connection *con) {
	uint8_t cmd = con->hdr->cmd;
	if ((cmd >= MEMCACHED_BIN_CMD_SET &&
	     cmd <= MEMCACHED_BIN_CMD_DECR) ||
	    (cmd >= MEMCACHED_BIN_CMD_APPEND &&
	     cmd <= MEMCACHED_BIN_CMD_PREPEND) ||
	    (cmd >= MEMCACHED_BIN_CMD_SETQ &&
	     cmd <= MEMCACHED_BIN_CMD_DECRQ) ||
	    (cmd >= MEMCACHED_BIN_CMD_APPENDQ &&
	     cmd <= MEMCACHED_BIN_CMD_PREPENDQ) ||
	    (cmd >= MEMCACHED_BIN_CMD_TOUCH &&
	     cmd <= MEMCACHED_BIN_CMD_GATQ) ||
	    (cmd >= MEMCACHED_BIN_CMD_GATK &&
	     cmd <= MEMCACHED_BIN_CMD_GATKQ))
		return 1;
	return 0;
};

static inline int
memcached_skip_request(struct memcached_connection *con) {
	struct ibuf *in = &con->iobuf->in;
	while (ibuf_used(in) < con->len && con->noprocess) {
		con->len -= ibuf_used(in);
		ibuf_reset(in);
		ssize_t read = mnet_read_ibuf(con->fd, in, 1);
		if (read < 1)
			return -1;
		con->cfg->stat.bytes_read += read;
	}
	in->rpos += con->len;
	return 0;
}

static inline int
memcached_bin_request(struct memcached_connection *con) {
	if (con->noprocess)
		goto noprocess;
	try {
		/* Process message */
		con->noreply = false;
		if (memcached_need_txn(con) && con->cfg->intxn != 0) {
			box_txn_begin();
			con->cfg->intxn = 1;
		}
		if (con->hdr->cmd < MEMCACHED_BIN_CMD_MAX) {
			con->cfg->intxn = mc_handler[con->hdr->cmd](con);
			if (memcached_need_txn(con)) {
				if (con->cfg->intxn == 1) {
					box_txn_commit();
				} else if (con->cfg->intxn == -1) {
					box_txn_rollback();
				}
			}
		} else {
			memcached_bin_process_unknown(con);
		}
	} catch (Exception *e) {
		char errstr[256];
		snprintf(errstr, 256, "SERVER ERROR '%s'", e->errmsg);
		memcached_error(con, MEMCACHED_BIN_RES_SERVER_ERROR,
					errstr);
		e->log();
		throw;
	}
noprocess:
	con->write_end = obuf_create_svp(&con->iobuf->out);
	memcached_skip_request(con);
	return 0;
}

static inline __attribute__((unused)) void
memcached_dump_hdr(struct memcached_hdr *hdr)
{
	if (!hdr) return;
	say_debug("memcached package");
	say_debug("magic:     0x%" PRIX8,        hdr->magic);
	say_debug("cmd:       0x%" PRIX8,        hdr->cmd);
	if (hdr->key_len > 0)
		say_debug("key_len:   %" PRIu16, bswap_u16(hdr->key_len));
	if (hdr->ext_len > 0)
		say_debug("ext_len:   %" PRIu8,  hdr->ext_len);
	say_debug("tot_len:   %" PRIu32,         bswap_u32(hdr->tot_len));
	say_debug("opaque:    0x%" PRIX32,       bswap_u32(hdr->opaque));
	say_debug("cas:       %" PRIu64,         bswap_u64(hdr->cas));
}

/**
 * return -1 on error (forced closing of connection)
 * return  0 in everything ok
 * - if con->noprocess == 1 then skip execution
 * return >1 if we need more data
 */
static inline int
memcached_parse_request(struct memcached_connection *con) {
	struct obuf     *out = &con->iobuf->out; (void )out;
	struct ibuf      *in = &con->iobuf->in;
	const char *reqstart = in->rpos;
	/* Check that we have enough data for header */
	if (reqstart + sizeof(struct memcached_hdr) > in->wpos) {
		return sizeof(struct memcached_hdr) - (in->wpos - reqstart);
	}
	struct memcached_hdr *hdr = (struct memcached_hdr *)reqstart;
	uint32_t tot_len = bswap_u32(hdr->tot_len);
	con->len = sizeof(struct memcached_hdr) + tot_len;
	con->hdr = hdr;
	/* error while parsing */
	if (hdr->magic != MEMCACHED_BIN_REQUEST) {
		memcached_error(con, MEMCACHED_BIN_RES_EINVAL, NULL);
		say_error("Wrong magic, closing connection");
		return -1;
	} else if (bswap_u16(hdr->key_len) + hdr->ext_len > tot_len) {
		memcached_error(con, MEMCACHED_BIN_RES_EINVAL, NULL);
		say_error("Object sizes are not consistent, skipping package");
		con->noprocess = true;
		return 0;
	}
	const char *reqend = reqstart + sizeof(struct memcached_hdr) + tot_len;
	/* Check that we have enough data for body */
	if (reqend > in->wpos) {
		return (reqend - in->wpos);
	}
	hdr->key_len = bswap_u16(hdr->key_len);
	hdr->tot_len = tot_len;
	hdr->opaque  = bswap_u32(hdr->opaque);
	hdr->cas     = bswap_u64(hdr->cas);
	const char *pos = reqstart + sizeof(struct memcached_hdr);
	con->body.ext_len = hdr->ext_len;
	con->body.key_len = hdr->key_len;
	con->body.val_len = hdr->tot_len - (hdr->ext_len + hdr->key_len);
	if (tot_len > MEMCACHED_MAX_SIZE) {
		memcached_error(con, MEMCACHED_BIN_RES_E2BIG, NULL);
		say_error("Object is too big for cache, skipping package");
		con->noprocess = true;
		return 0;
	}
	if (con->body.ext_len > 0) {
		con->body.ext = pos;
		pos += hdr->ext_len;
	} else {
		con->body.ext = NULL;
	}
	if (con->body.key_len > 0) {
		con->body.key = pos;
		pos += hdr->key_len;
	} else {
		con->body.key = NULL;
	}
	if (con->body.val_len > 0) {
		con->body.val = pos;
		pos += con->body.val_len;
	} else {
		con->body.val = NULL;
	}
	assert(pos == reqend);
	return 0;
}

static inline ssize_t
memcached_flush(struct memcached_connection *con) {
	struct iobuf *iobuf = con->iobuf;
	ssize_t total = mnet_writev(con->fd, iobuf->out.iov,
				    obuf_iovcnt(&iobuf->out),
				    obuf_size(&iobuf->out));
	con->cfg->stat.bytes_written += total;
	if (ibuf_used(&iobuf->in) == 0)
		ibuf_reset(&iobuf->in);
	obuf_reset(&iobuf->out);
	ibuf_reserve_nothrow(&iobuf->in, con->cfg->readahead);
	return total;
}

static inline void
memcached_loop(struct memcached_connection *con)
{
	struct iobuf *iobuf = con->iobuf;
	int rc = 0;
	struct ibuf *in = &iobuf->in;
	size_t to_read = 24;
	int batch_count = 0;

	for (;;) {
		ibuf_reserve_nothrow(&iobuf->in, to_read);
		ssize_t read = mnet_read_ibuf(con->fd, in, to_read);
		if (read < to_read)
			break;
		con->cfg->stat.bytes_read += read;
		to_read = 24;
next:
		rc = memcached_parse_request(con);
		if (rc == -1) {
			/* We close connection, because of wrong magic */
			break;
		} else if (rc > 0) {
			to_read = rc;
			continue;
		}
		memcached_bin_request(con);
		if (con->close_connection) {
			say_debug("Requesting exit. Exiting.");
			break;
		} else if (rc == 0 && ibuf_used(in) > 0 &&
			   batch_count < con->cfg->batch_count) {
			/* Need to add check for batch count */
			batch_count++;
			goto next;
		}
		batch_count = 0;
		/* Write back answer */
		if (!con->noreply)
			memcached_flush(con);
		fiber_gc();
		con->noreply = false;
		con->noprocess = false;
	}
	memcached_flush(con);
}

void
memcached_handler(struct memcached_service *p, int fd)
{
	struct iobuf    *iobuf      = iobuf_new();

	struct memcached_connection con;
	/* TODO: move to connection_init */
	memset(&con, 0, sizeof(struct memcached_connection));
	con.fd        = fd;
	con.iobuf     = iobuf;
	con.write_end = obuf_create_svp(&iobuf->out);
//	memcpy(&con.addr, addr, addr_len);
//	con.addr_len  = addr_len;
//	con.session   = session_create(con.fd, *(uint64_t *)&con.addr);
	con.cfg       = p;

	/* read-write cycle */
	con.cfg->stat.curr_conns++;
	con.cfg->stat.total_conns++;
//	con.cfg->stat.started = fiber_time64();
	try {
		auto scoped_guard = make_scoped_guard([&] {
			con.cfg->stat.curr_conns--;
			close(con.fd);
			iobuf_delete(iobuf);
		});

		memcached_loop(&con);
		return;
	} catch (const FiberCancelException& e) {
		throw;
	} catch (const Exception& e) {
		e.log();
	}
}

int
memcached_expire_process(struct memcached_service *p, box_iterator_t *iter)
{
	box_tuple_t *tpl = NULL;
	box_txn_begin();
	for (int i = 0; i < p->expire_count; ++i) {
		if (box_iterator_next(iter, &tpl)) {
			box_txn_rollback();
			return -1;
		} else if (tpl == NULL) {
			box_txn_commit();
			return -2;
		} else if (is_expired_tuple(p, tpl)) {
			uint32_t klen = 0;
			const char *kpos = box_tuple_field(tpl, 0);
			kpos = mp_decode_str(&kpos, &klen);
			char *begin = (char *)box_txn_alloc(mp_sizeof_array(1) +
							    mp_sizeof_str(klen));
			char *end = mp_encode_array(begin, 1);
			      end = mp_encode_str(end, kpos, klen);
			if (box_delete(p->space_id, 0, begin, end, NULL)) {
				box_txn_rollback();
				return -1;
			}
			p->stat.evictions++;
		}
	}
	box_txn_commit();
	return 0;
}

void
memcached_expire_loop(va_list ap)
{
	struct memcached_service *p = va_arg(ap, struct memcached_service *);
	char key[2], *key_end = mp_encode_array(key, 0);
	box_iterator_t *iter = NULL;
	int rv = 0;
	auto scoped_guard = make_scoped_guard([&] {
		if (iter) box_iterator_free(iter);
	});
	say_info("Memcached expire fiber started");
	iter = box_index_iterator(p->space_id, 0, ITER_ALL, key, key_end);
restart:
	rv = memcached_expire_process(p, iter);
	if (rv == -1) {
		const box_error_t *err = box_error_last();
		say_error("Unexpected error %u: %s",
				box_error_code(err),
				box_error_message(err));
		return;
	} else if (rv == -2) {
		if (iter) box_iterator_free(iter);
		iter = box_index_iterator(p->space_id, 0, ITER_ALL, key, key_end);
	}

	/* This part is where we rest after all deletes */
	double delay = ((double )p->expire_count * p->expire_time) /
			(box_index_len(p->space_id, 0) + 1);
	if (delay > 1) delay = 1;
	fiber_set_cancellable(true);
	fiber_sleep(delay);
	fiber_set_cancellable(false);
	fiber_gc();

	goto restart;
}

void
memcached_expire_start(struct memcached_service *p)
{
	if (p->expire_fiber != NULL) return;
	struct fiber *expire_fiber = NULL;
	char name[128];
	snprintf(name, 128, "%s_memcached_expire", p->name);
	try {
		expire_fiber = fiber_new(name, memcached_expire_loop);
	} catch (const Exception& e) {
		say_error("Can't start the expire fiber");
		e.log();
		return;
	}
	p->expire_fiber = expire_fiber;
	fiber_start(expire_fiber, p);
}

void
memcached_expire_stop(struct memcached_service *p)
{
	if (p->expire_fiber == NULL) return;
	fiber_cancel(p->expire_fiber);
	fiber_join(p->expire_fiber);
	p->expire_fiber = NULL;
}

struct memcached_service*
memcached_create(const char *name, uint32_t sid)
{
	struct memcached_service *srv = (struct memcached_service *)calloc(1,
			sizeof(struct memcached_service));
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
	free (srv);
}

void
memcached_start (struct memcached_service *srv)
{
	memcached_expire_start(srv);
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
		srv->readahead = va_arg(va, int);
		break;
	case MEMCACHED_OPT_EXPIRE_ENABLED: {
		int flag = va_arg(va, int);
		if (flag == 0) {
			srv->expire_enabled = false;
			memcached_expire_stop(srv);
		} else {
			srv->expire_enabled = true;
		}
		break;
	}
	case MEMCACHED_OPT_EXPIRE_COUNT:
		srv->expire_count = va_arg(va, uint32_t);
		break;
	case MEMCACHED_OPT_EXPIRE_TIME:
		srv->expire_time = va_arg(va, uint32_t);
		break;
	case MEMCACHED_OPT_FLUSH_ENABLED: {
		int flag = va_arg(va, int);
		if (flag == 0) {
			srv->flush_enabled = false;
		} else {
			srv->flush_enabled = true;
		}
		break;
	}
	case MEMCACHED_OPT_VERBOSITY: {
		int flag = va_arg(va, int);
		if (flag > 0) {
			srv->verbosity = (flag < 4 ? flag : 3);
		} else if (flag > 3) {
			srv->verbosity = 0;
		}
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
