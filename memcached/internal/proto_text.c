#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <tarantool/module.h>
#include <msgpuck.h>
#include <small/ibuf.h>
#include <small/obuf.h>

#include "memcached.h"
#include "constants.h"
#include "memcached_layer.h"
#include "error.h"
#include "utils.h"
#include "proto_text.h"
#include "proto_text_parser.h"

#define memcached_text_DUP(_con, _msg, _len) do {				  \
	if (!(_con)->noreply && obuf_dup((_con)->out, (_msg), (_len)) != (_len)) {\
		/* if ((_svp)) obuf_rollback_to_svp(con->out, (_svp)); */	  \
		memcached_error_ENOMEM((_len), "obuf_dup");			  \
		return -1;							  \
	}									  \
} while (0)

static inline int
txt_get_single(struct memcached_connection *con, const char *key,
	       size_t key_len, bool get_cas)
{
	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, key, key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}
	
	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		con->cfg->stat.get_misses++;
		return 1;
	}
	uint32_t vlen = 0, klen = 0;
	const char *pos  = box_tuple_field(tuple, 0);
	const char *kpos = mp_decode_str(&pos, &klen);
	mp_next(&pos); /* skip expiration time */
	mp_next(&pos); /* skip creation time */
	const char *vpos = mp_decode_str(&pos, &vlen);
	uint64_t cas     = mp_decode_uint(&pos);
	uint32_t flags   = mp_decode_uint(&pos);

	/* Prepare end of first line */
	char end[128] = {0};
	size_t elen = 0;
	elen = snprintf((char *)end, 128,
			" %" PRIu32 " %" PRIu32,
			flags, vlen);
	if (get_cas == true) {
		elen += snprintf((char *)end + elen,
				 128 - elen,
				 " %" PRIu64,
				 cas);
	}
	end[elen++] = '\r';
	end[elen++] = '\n';

	size_t len = 8 + klen + elen + vlen;
	if (obuf_reserve(con->out, len) == NULL) {
		memcached_error_ENOMEM(len, "obuf");
		return -1;
	}

	if (obuf_dup(con->out, "VALUE ", 6) != 6 ||
	    obuf_dup(con->out, kpos,  klen) != klen ||
	    obuf_dup(con->out, end,   elen) != elen ||
	    obuf_dup(con->out, vpos,  vlen) != vlen ||
	    obuf_dup(con->out, "\r\n",   2) != 2) {
		/* unreachable */
		assert(0);
	}

	con->cfg->stat.get_hits++;
	return 0;
}

int
memcached_txt_process_get(struct memcached_connection *con)
{
	char *key = (char *)con->request.key;
	char *key_end = key + con->request.key_len;
	char *tmp_begin = key, *tmp_end = key;
	bool get_cas = (con->request.op == MEMCACHED_TXT_CMD_GETS);

	struct obuf_svp svp = obuf_create_svp(con->out);
	do {
		/* find end of key */
		for (; *tmp_end != ' ' && *tmp_end != '\r' &&
		       *tmp_end != '\n'; ++tmp_end);

		int rv = txt_get_single(con, tmp_begin, tmp_end - tmp_begin,
					get_cas);
		if (rv < 0) goto error;

		/* skip whitespaces */
		for (; *tmp_end == ' '; ++tmp_end);
		if (tmp_end >= key_end) break;
		tmp_begin = tmp_end;
	} while (1);

	if (obuf_dup(con->out, "END\r\n", 5) != 5) {
		memcached_error_ENOMEM(5, "obuf");
		goto error;
	}
	return 0;
error:
	obuf_rollback_to_svp(con->out, &svp);
	return -1;
}

int
memcached_txt_process_set(struct memcached_connection *con)
{
	int               cmd = con->request.op;
	uint64_t cas_expected = 0;
	const char       *key = con->request.key;
	size_t        key_len = con->request.key_len;
	if (cmd == MEMCACHED_TXT_CMD_CAS)
		cas_expected = con->request.cas;
	
	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, key, key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}
	
	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	/* Check for key (non)existence for different commands */
	if (cmd == MEMCACHED_TXT_CMD_REPLACE && (!tuple_exists || tuple_expired)) {
		memcached_text_DUP(con, "NOT_STORED\r\n", 12);
		return 0;
	} else if (cmd == MEMCACHED_TXT_CMD_ADD && tuple_exists) {
		if (!tuple_expired) {
			memcached_text_DUP(con, "NOT_STORED\r\n", 12);
			return 0;
		}
		con->cfg->stat.reclaimed++;
	} else if (cmd == MEMCACHED_TXT_CMD_CAS) {
		if (!tuple_exists || tuple_expired) {
			con->cfg->stat.cas_misses++;
			memcached_text_DUP(con, "NOT_FOUND\r\n", 11);
			return 0;
		}
		const char *pos   = box_tuple_field(tuple, 4);
		assert(mp_typeof(*pos) == MP_UINT);
		uint64_t cas_prev = mp_decode_uint(&pos);
		if (cas_prev != cas_expected) {
			con->cfg->stat.cas_badval++;
			memcached_text_DUP(con, "EXISTS\r\n", 8);
			return 0;
		}
		con->cfg->stat.cas_hits++;
	}
	uint32_t      flags = con->request.flags;
	uint64_t    exptime = convert_exptime(con->request.exptime);
	const char   *value = con->request.data;
	size_t    value_len = con->request.data_len;
	uint64_t new_cas = con->cfg->cas++;

	if (memcached_tuple_set(con, key, key_len, exptime, value,
				value_len, new_cas, flags) == -1) {
		box_txn_rollback();
		return -1;
	}
	memcached_text_DUP(con, "STORED\r\n", 8);
	return 0;
}

int
memcached_txt_process_delta(struct memcached_connection *con)
{
	const char    *key = con->request.key;
	size_t     key_len = con->request.key_len;

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, key, key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	uint64_t val = 0;
	uint64_t new_cas = con->cfg->cas++;
	char     strval[23]; uint8_t strvallen = 0;

	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		memcached_text_DUP(con, "NOT_FOUND\r\n", 11);
		return 0;
	}

	uint32_t vlen = 0;//, klen = 0;
	const char *pos  = box_tuple_field(tuple, 1);
	/*const char *kpos = mp_decode_str(&pos, &klen);*/
	uint64_t expire  = mp_decode_uint(&pos);
	mp_next(&pos); /* skip creation time */
	const char *vpos = mp_decode_str(&pos, &vlen);
	mp_next(&pos); /* skip cas */
	uint32_t flags   = mp_decode_uint(&pos);


	uint64_t delta = con->request.delta;

	if (memcached_strtoul(vpos, vpos + vlen, &val) == -1) {
		say_error("Bad value for delta operation: \"%.*s\"", vlen, vpos);
		memcached_error(MEMCACHED_RES_DELTA_BADVAL);
		return -1;
	}
	if (con->request.op == MEMCACHED_TXT_CMD_INCR) {
		uint64_t val_prev = val;
		val += delta;
		if (val < val_prev) val = val_prev;
	} else if (delta > val) {
		val = 0;
	} else {
		val -= delta;
	}

	/* Insert value */
	strvallen = snprintf(strval, 22, "%" PRIu64, val);
	if (memcached_tuple_set(con, key, key_len, expire, (const char *)strval,
				strvallen, new_cas, flags) == -1) {
		box_txn_rollback();
		return -1;
	}
	strval[strvallen++] = '\r';
	strval[strvallen++] = '\n';
	memcached_text_DUP(con, strval, strvallen);

	return 0;
}

int
memcached_txt_process_pend(struct memcached_connection *con)
{
	const char    *key = con->request.key;
	size_t     key_len = con->request.key_len;
	const char   *data = con->request.data;
	size_t    data_len = con->request.data_len;

	con->cfg->stat.cmd_set++;
	uint64_t new_cas = con->cfg->cas++;

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, key, key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		memcached_text_DUP(con, "NOT_STORED\r\n", 12);
		return 0;
	}

	uint32_t    vlen = 0, klen = 0, flags = 0;
	const char *kpos = NULL, *vpos = NULL;
	uint64_t exptime = 0;
	const char *pos  = box_tuple_field(tuple, 0);
	kpos             = mp_decode_str(&pos, &klen);
	exptime          = mp_decode_uint(&pos);
	mp_next(&pos); /* skip creation time */
	vpos             = mp_decode_str(&pos, &vlen);
	mp_next(&pos); /* skip cas */
	flags            = mp_decode_uint(&pos);

	char *begin = (char *)box_txn_alloc(data_len + vlen);
	if (begin == NULL) {
		memcached_error_ENOMEM(data_len + vlen, "value");
		return -1;
	}
	if (con->request.op == MEMCACHED_TXT_CMD_PREPEND) {
		memcpy(begin, data, data_len);
		memcpy(begin + data_len, vpos, vlen);
	} else {
		memcpy(begin, vpos, vlen);
		memcpy(begin + vlen, data, data_len);
	}

	exptime = convert_exptime(con->request.exptime);

	/* Tuple can't be NULL, because we already found this element */
	if (memcached_tuple_set(con, kpos, klen, exptime, begin,
				vlen + data_len, new_cas, flags) == -1) {
		box_txn_rollback();
		return -1;
	}
	memcached_text_DUP(con, "STORED\r\n", 8);
	return 0;
}

int
memcached_txt_process_delete(struct memcached_connection *con)
{
	char      *key = (char *)con->request.key;
	size_t key_len = con->request.key_len;

	con->cfg->stat.cmd_delete++;
	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len, "key");
		return -1;
	}
	char *end   = mp_encode_array(begin, 1);
	      end   = mp_encode_str  (end, key, key_len);
	assert(end <= begin + len);
	box_tuple_t *tuple = NULL;
	if (box_delete(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	/* We didn't delete anything, or tuple is already expired */
	if (!tuple_exists || tuple_expired) {
		if (tuple_expired) con->cfg->stat.evictions++;
		con->cfg->stat.delete_misses++;
		memcached_text_DUP(con, "NOT_FOUND\r\n", 11);
	} else {
		con->cfg->stat.delete_hits++;
		memcached_text_DUP(con, "DELETED\r\n", 9);
	}
	return 0;
}

int
memcached_txt_process_flush(struct memcached_connection *con)
{
	con->cfg->stat.cmd_flush++;
	uint64_t exptime = con->request.exptime;
	con->cfg->flush = fiber_time64();
	if (exptime > 0) con->cfg->flush = convert_exptime(exptime);
	memcached_text_DUP(con, "OK\r\n", 4);
	return 0;
}

int
memcached_txt_process_version(struct memcached_connection *con)
{
	char value[256] = {0};
	size_t value_len = snprintf((char *)value, 256, "VERSION %s\r\n",
				    PACKAGE_VERSION);
	memcached_text_DUP(con, (char *)value, value_len);
	return 0;
}

int
memcached_txt_process_verbosity(struct memcached_connection *con)
{
	int verbosity = con->request.exptime;
	if (verbosity <= 3) {
		con->cfg->verbosity = verbosity;
	} else {
		memcached_error(MEMCACHED_RES_EINVAL);
		return -1;
	}
	memcached_text_DUP(con, "OK\r\n", 4);
	return 0;
}

int
memcached_txt_process_quit(struct memcached_connection *con)
{
	con->close_connection = true;
	return 0;
}

static inline int
stat_append(struct memcached_connection *con, const char *key,
	    const char *valfmt, ...)
{
	struct obuf *out = con->out;
	if (!key) {
		if (obuf_reserve(out, 5) == NULL) {
			memcached_error_ENOMEM(5, "obuf");
			return -1;
		}
		if (obuf_dup(out, "END", 3) != 3) {
			/* unreachable */
			assert(0);
		}
	} else {
		char val_tmp[256] = {0};
		va_list va;
		va_start(va, valfmt);
		size_t val_len = vsnprintf((char *)val_tmp + 1, 255, valfmt, va);
		val_tmp[0] = ' '; ++val_len;
		va_end(va);

		uint32_t key_len = strlen(key);
		size_t len = 8 + key_len + val_len;
		if (obuf_reserve(out, len) == NULL) {
			memcached_error_ENOMEM(len, "obuf");
			return -1;
		}
		if (obuf_dup(out, "STAT ", 5) != 5 ||
		    obuf_dup(out, key, key_len) != key_len ||
		    obuf_dup(out, (const char *)val_tmp, val_len) != val_len) {
			/* unreachable */
			assert(0);
		}
	}
	if (obuf_dup(out, "\r\n", 2) != 2) {
		/* unreachable */
		assert(0);
	}
	return 0;
}

int
memcached_txt_process_stat(struct memcached_connection *con) {

	/* default declarations */
	struct memcached_text_request *req = &con->request;
	stat_func_t append = stat_append;
	struct obuf_svp svp = obuf_create_svp(con->out);

	/* ADD errstr for TODO */
	if (req->key_len == 0) {
		if (memcached_stat_all(con, append) == -1)
			goto error;
	} else if (req->key_len == 5  && !strncmp(req->key, "reset", 5)) {
		if (memcached_stat_reset(con, append) == -1)
			goto error;
/*	} else if (req->key_len == 6  && !strncmp(req->key, "detail", 6)) {
		memcached_error_NOT_SUPPORTED("stat detail");
		return -1;
	} else if (req->key_len == 11 && !strncmp(req->key, "detail dump", 11)) {
		memcached_error_NOT_SUPPORTED("stat detail dump");
		return -1;
	} else if (req->key_len == 9  && !strncmp(req->key, "detail on", 9)) {
		memcached_error_NOT_SUPPORTED("stat detail on");
		return -1;
	} else if (req->key_len == 10 && !strncmp(req->key, "detail off", 10)) {
		memcached_error_NOT_SUPPORTED("stat detail off");
		return -1;*/
	} else {
		char err[256] = {0};
		snprintf(err, 256, "stat %.*s", (int )req->key_len, req->key);
		memcached_error_NOT_SUPPORTED(err);
		return -1;
	}
	return 0;
error:
	obuf_rollback_to_svp(con->out, &svp);
	return -1;
}

int
memcached_txt_process_unsupported(struct memcached_connection *con)
{
	memcached_error_NOT_SUPPORTED(memcached_txt_cmdname(con->request.op));
	return -1;
}

int
memcached_txt_process_unknown(struct memcached_connection *con)
{
	memcached_error_UNKNOWN_COMMAND(con->request.op);
	return -1;
}

const mc_process_func_t memcached_txt_handler[] = {
	memcached_txt_process_unknown,   /* RESERVED,                  0x00 */
	memcached_txt_process_set,       /* MEMCACHED_TXT_CMD_SET,     0x01 */
	memcached_txt_process_set,       /* MEMCACHED_TXT_CMD_ADD,     0x02 */
	memcached_txt_process_set,       /* MEMCACHED_TXT_CMD_REPLACE, 0x03 */
	memcached_txt_process_pend,      /* MEMCACHED_TXT_CMD_APPEND,  0x04 */
	memcached_txt_process_pend,      /* MEMCACHED_TXT_CMD_PREPEND, 0x05 */
	memcached_txt_process_set,       /* MEMCACHED_TXT_CMD_CAS,     0x06 */
	memcached_txt_process_get,       /* MEMCACHED_TXT_CMD_GET,     0x07 */
	memcached_txt_process_get,       /* MEMCACHED_TXT_CMD_GETS,    0x08 */
	memcached_txt_process_delete,    /* MEMCACHED_TXT_CMD_DELETE,  0x09 */
	memcached_txt_process_delta,     /* MEMCACHED_TXT_CMD_INCR,    0x0a */
	memcached_txt_process_delta,     /* MEMCACHED_TXT_CMD_DECR,    0x0b */
	memcached_txt_process_flush,     /* MEMCACHED_TXT_CMD_FLUSH,   0x0c */
	memcached_txt_process_stat,      /* MEMCACHED_TXT_CMD_STATS,   0x0d */
	memcached_txt_process_version,   /* MEMCACHED_TXT_CMD_VERSION, 0x0e */
	memcached_txt_process_quit,      /* MEMCACHED_TXT_CMD_QUIT,    0x0f */
	memcached_txt_process_verbosity, /* MEMCACHED_TXT_CMD_VERSION, 0x0e */
	NULL
};

/**
 * Check that we need transaction for our operation.
 */
static inline int
memcached_text_ntxn(struct memcached_connection *con)
{
	uint8_t cmd = con->request.op;
	if ((cmd <= MEMCACHED_TXT_CMD_CAS) ||
	    (cmd >= MEMCACHED_TXT_CMD_DELETE &&
	     cmd <= MEMCACHED_TXT_CMD_DECR))
		return 1;
	return 0;
};

int
memcached_text_process(struct memcached_connection *con)
{
	int rv = 0;
	/* Process message */
	if (memcached_text_ntxn(con))
		box_txn_begin();
	if (con->request.op < MEMCACHED_TXT_CMD_MAX) {
		rv = memcached_txt_handler[con->request.op](con);
		if (box_txn()) box_txn_commit();
	} else {
		rv = memcached_process_unknown(con);
	}
	return rv;
}

int
memcached_text_parse(struct memcached_connection *con)
{
	struct ibuf      *in = con->in;
	const char *reqstart = in->rpos, *end = in->wpos;
	int rv = memcached_text_parser(con, &reqstart, end);
	if (reqstart > in->rpos)
		con->len = reqstart - in->rpos;
	if (rv == 0)
		con->noreply = con->request.noreply;
	return rv;
}

/**
 * Each command sent by a client may be answered with an error string
 * from the server. These error strings come in three types:
 *
 * - "ERROR\r\n"
 *
 *   means the client sent a nonexistent command name.
 *
 * - "CLIENT_ERROR <error>\r\n"
 *
 *   means some sort of client error in the input line, i.e. the input
 *   doesn't conform to the protocol in some way.
 *   <error> is a human-readable error string.
 *
 * - "SERVER_ERROR <error>\r\n"
 *
 *   means some sort of server error prevents the server from carrying
 *   out the command. In cases of severe server errors, which make it
 *   impossible to continue serving the client (this shouldn't normally
 *   happen), the server will close the connection after sending the error
 *   line. This is the only case in which the server closes a connection
 *   to a client.
 *   <error> is a human-readable error string.
 */

int
memcached_text_error(struct memcached_connection *con,
		     uint16_t err, const char *errstr)
{
	struct obuf *out = con->out;
	if (!errstr) {
		say_info("0x%x", err);
		errstr = memcached_get_result_description(err);
		if (!errstr) {
			say_error("Unknown errcode - 0x%.2X", err);
			errstr = "UNKNOWN ERROR";
		}
	}
	size_t len = strlen(errstr);
	if (err == MEMCACHED_RES_NOT_SUPPORTED ||
	    err == MEMCACHED_RES_UNKNOWN_COMMAND) {
		obuf_dup(out, "ERROR", 5);
	} else if (err == MEMCACHED_RES_EINVAL       ||
		   err == MEMCACHED_RES_DELTA_BADVAL) {
		/* TODO: add error type support */
		obuf_dup(out, "CLIENT_ERROR ", 13);
		obuf_dup(out, errstr, len);
	} else {
		obuf_dup(out, "SERVER_ERROR ", 13);
		obuf_dup(out, errstr, len);
	}
	obuf_dup(out, "\r\n", 2);
	return 0;
}

void
memcached_set_text(struct memcached_connection *con)
{
	con->cb.parse_request   = memcached_text_parse;
	con->cb.process_request = memcached_text_process;
	con->cb.process_error   = memcached_text_error;
}

#undef memcached_text_DUP
