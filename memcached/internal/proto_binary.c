#include <inttypes.h>
#include <stdio.h>

#include <tarantool.h>
#include <msgpuck/msgpuck.h>

#include "error.h"
#include "memcached.h"
#include "constants.h"
#include "memcached_layer.h"

#include "proto_binary.h"

#include <small/ibuf.h>
#include <small/obuf.h>

const mc_process_func_t memcached_bin_handler[] = {
	memcached_bin_process_get,         /* MEMCACHED_BIN_CMD_GET      , 0x00 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_SET      , 0x01 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_ADD      , 0x02 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_REPLACE  , 0x03 */
	memcached_bin_process_delete,      /* MEMCACHED_BIN_CMD_DELETE   , 0x04 */
	memcached_bin_process_delta,       /* MEMCACHED_BIN_CMD_INCR     , 0x05 */
	memcached_bin_process_delta,       /* MEMCACHED_BIN_CMD_DECR     , 0x06 */
	memcached_bin_process_quit,        /* MEMCACHED_BIN_CMD_QUIT     , 0x07 */
	memcached_bin_process_flush,       /* MEMCACHED_BIN_CMD_FLUSH    , 0x08 */
	memcached_bin_process_get,         /* MEMCACHED_BIN_CMD_GETQ     , 0x09 */
	memcached_bin_process_noop,        /* MEMCACHED_BIN_CMD_NOOP     , 0x0a */
	memcached_bin_process_version,     /* MEMCACHED_BIN_CMD_VERSION  , 0x0b */
	memcached_bin_process_get,         /* MEMCACHED_BIN_CMD_GETK     , 0x0c */
	memcached_bin_process_get,         /* MEMCACHED_BIN_CMD_GETKQ    , 0x0d */
	memcached_bin_process_pend,        /* MEMCACHED_BIN_CMD_APPEND   , 0x0e */
	memcached_bin_process_pend,        /* MEMCACHED_BIN_CMD_PREPEND  , 0x0f */
	memcached_bin_process_stat,        /* MEMCACHED_BIN_CMD_STAT     , 0x10 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_SETQ     , 0x11 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_ADDQ     , 0x12 */
	memcached_bin_process_set,         /* MEMCACHED_BIN_CMD_REPLACEQ , 0x13 */
	memcached_bin_process_delete,      /* MEMCACHED_BIN_CMD_DELETEQ  , 0x14 */
	memcached_bin_process_delta,       /* MEMCACHED_BIN_CMD_INCRQ    , 0x15 */
	memcached_bin_process_delta,       /* MEMCACHED_BIN_CMD_DECRQ    , 0x16 */
	memcached_bin_process_quit,        /* MEMCACHED_BIN_CMD_QUITQ    , 0x17 */
	memcached_bin_process_flush,       /* MEMCACHED_BIN_CMD_FLUSHQ   , 0x18 */
	memcached_bin_process_pend,        /* MEMCACHED_BIN_CMD_APPENDQ  , 0x19 */
	memcached_bin_process_pend,        /* MEMCACHED_BIN_CMD_PREPENDQ , 0x1a */
	memcached_bin_process_verbosity,   /* MEMCACHED_BIN_CMD_VERBOSITY, 0x1b */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_TOUCH    , 0x1c */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GAT      , 0x1d */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GATQ     , 0x1e */
	memcached_process_unknown,         /* RESERVED                   , 0x1f */
	memcached_process_unsupported,     /* MEMCACHED_._SASL_LIST_MECHS, 0x20 */
	memcached_process_unsupported,     /* MEMCACHED_._SASL_AUTH      , 0x21 */
	memcached_process_unsupported,     /* MEMCACHED_._SASL_STEP      , 0x22 */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GATK     , 0x23 */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GATKQ    , 0x24 */
	memcached_process_unknown,         /* RESERVED                   , 0x25 */
	memcached_process_unknown,         /* RESERVED                   , 0x26 */
	memcached_process_unknown,         /* RESERVED                   , 0x27 */
	memcached_process_unknown,         /* RESERVED                   , 0x28 */
	memcached_process_unknown,         /* RESERVED                   , 0x29 */
	memcached_process_unknown,         /* RESERVED                   , 0x2a */
	memcached_process_unknown,         /* RESERVED                   , 0x2b */
	memcached_process_unknown,         /* RESERVED                   , 0x2c */
	memcached_process_unknown,         /* RESERVED                   , 0x2d */
	memcached_process_unknown,         /* RESERVED                   , 0x2e */
	memcached_process_unknown,         /* RESERVED                   , 0x2f */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RGET     , 0x30 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RSET     , 0x31 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RSETQ    , 0x32 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RAPPEND  , 0x33 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RAPPENDQ , 0x34 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RPREPEND , 0x35 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RPREPENDQ, 0x36 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RDELETE  , 0x37 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RDELETEQ , 0x38 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RINCR    , 0x39 */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RINCRQ   , 0x3a */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RDECR    , 0x3b */
	memcached_process_unsupported,     /* MEMCACHED_BIN_CMD_RDECRQ   , 0x3c */
};

int
memcached_binary_write(struct memcached_connection *con, uint16_t err,
		       uint64_t cas, uint8_t ext_len, uint16_t key_len,
		       uint32_t val_len, const char *ext,
		       const char *key, const char *val)
{
	assert((ext && ext_len > 0) || (!ext && ext_len == 0));
	assert((key && key_len > 0) || (!key && key_len == 0));
	assert((val && val_len > 0) || (!val && val_len == 0));
	struct memcached_hdr *hdri = con->hdr;
	struct obuf *out = con->out;

	struct memcached_hdr hdro;
	memcpy(&hdro, hdri, sizeof(struct memcached_hdr));
	hdro.magic   = MEMCACHED_BIN_RESPONSE;
	hdro.ext_len = ext_len;
	hdro.key_len = mp_bswap_u16(key_len);
	hdro.status  = mp_bswap_u16(err);
	hdro.tot_len = mp_bswap_u32(ext_len + key_len + val_len);
	hdro.opaque  = mp_bswap_u32(hdro.opaque);
	hdro.cas     = mp_bswap_u64(cas);
	size_t to_alloc = ext_len + key_len + val_len +
			sizeof(struct memcached_hdr);
	if (obuf_reserve(out, to_alloc) == NULL) {
		memcached_error_ENOMEM(to_alloc, "write_output", "obuf");
		return -1;
	}
	size_t rv = 0;
	rv = obuf_dup(out, &hdro, sizeof(struct memcached_hdr));
	assert(rv == sizeof(struct memcached_hdr));
	if (ext) {
		rv = obuf_dup(out, ext, ext_len);
		assert(rv == ext_len);
	}
	if (key) {
		rv = obuf_dup(out, key, key_len);
		assert(rv == key_len);
	}
	if (val) {
		rv = obuf_dup(out, val, val_len);
		assert(rv == val_len);
	}
	return 0;
}

/**
 * Check that we need transaction for our operation.
 */
static inline int
memcached_binary_ntxn(struct memcached_connection *con)
{
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

int
memcached_binary_process(struct memcached_connection *con)
{
	int rv = 0;
	/* Process message */
	con->noreply = false;
	if (memcached_binary_ntxn(con)) {
		box_txn_begin();
	}
	if (con->hdr->cmd < MEMCACHED_BIN_CMD_MAX) {
		rv = memcached_bin_handler[con->hdr->cmd](con);
		if (box_txn()) box_txn_commit();
	} else {
		rv = memcached_process_unknown(con);
	}
	return rv;
}

/**
 * return -1 on error (forced closing of connection)
 * return  0 in everything ok
 * - if con->noprocess == 1 then skip execution
 * return >1 if we need more data
 */
int
memcached_binary_parse(struct memcached_connection *con)
{
	struct ibuf      *in = con->in;
	const char *reqstart = in->rpos;
	/* Check that we have enough data for header */
	if (reqstart + sizeof(struct memcached_hdr) > in->wpos) {
		return sizeof(struct memcached_hdr) - (in->wpos - reqstart);
	}
	struct memcached_hdr *hdr = (struct memcached_hdr *)reqstart;
	uint32_t tot_len = mp_bswap_u32(hdr->tot_len);
	con->len = sizeof(struct memcached_hdr) + tot_len;
	con->hdr = hdr;
	/* error while parsing */
	if (hdr->magic != MEMCACHED_BIN_REQUEST) {
		memcached_error_EINVAL();
		say_error("Wrong magic, closing connection");
		con->close_connection = true;
		return -1;
	} else if (mp_bswap_u16(hdr->key_len) + hdr->ext_len > tot_len) {
		memcached_error_EINVAL();
		con->noprocess = true;
		say_error("Object sizes are not consistent, skipping package");
		return -1;
	}
	const char *reqend = reqstart + sizeof(struct memcached_hdr) + tot_len;
	/* Check that we have enough data for body */
	if (reqend > in->wpos) {
		return (reqend - in->wpos);
	}
	hdr->key_len = mp_bswap_u16(hdr->key_len);
	hdr->tot_len = tot_len;
	hdr->opaque  = mp_bswap_u32(hdr->opaque);
	hdr->cas     = mp_bswap_u64(hdr->cas);
	const char *pos = reqstart + sizeof(struct memcached_hdr);
	con->body.ext_len = hdr->ext_len;
	con->body.key_len = hdr->key_len;
	con->body.val_len = hdr->tot_len - (hdr->ext_len + hdr->key_len);
	if (tot_len > MEMCACHED_MAX_SIZE) {
		memcached_error_E2BIG();
		say_error("Object is too big for cache, skipping package");
		con->noprocess = true;
		return -1;
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

/**
 * Write error code message to output buffer
 *
 * \param[in] con    Connection object for writing output to
 * \param[in] err    Error code (status)
 * \param[in] errstr Eror string (if available) with description (may be NULL)
 *
 */
int
memcached_binary_error(struct memcached_connection *con,
		       uint16_t err, const char *errstr)
{
	if (!errstr) {
		errstr = memcached_get_result_title(err);
		if (!errstr) {
			say_error("Unknown errcode - 0x%.2X", err);
			errstr = "UNKNOWN ERROR";
		}
	}
	say_error("memcached error %" PRIu16 ": %s", err, errstr);
	size_t len = 0;
	if (errstr) len = strlen(errstr);
	if (con->cb.write_answer(con, err, 0, 0, 0, len, NULL, NULL, errstr) == -1)
		return -1;
	return 0;
}

int
memcached_binary_stat(struct memcached_connection *con, const char *key,
		      const char *valfmt, ...)
{
	size_t key_len = 0;
	if (key) key_len = strlen(key);
	size_t val_len = 0;
	char val_tmp[256] = {0};
	char *val = NULL;
	if (valfmt) {
		va_list va;
		va_start(va, valfmt);
		val_len = vsnprintf(val_tmp, 256, valfmt, va);
		va_end(va);
		val = val_tmp;
	} else {
		val = NULL;
	}
	if (con->cb.write_answer(con, 0, 0, 0, key_len,
			 val_len, NULL, key, val) == -1)
		return -1;
	return 0;
};

#if 0
/**
 * Process internal Tarantool error
 * (get exception from diag_area and write it out)
 *
 * \param[in] con    Connection object for writing output to
 */
int memcached_bin_errori(struct memcached_connection *con)
{
	const box_error_t  *err = box_error_last();
	uint16_t        errcode = box_error_code(err);
	char       errfstr[256] = {0};
	const char      *errstr = box_error_message(err);
	int rv = 0;
	switch(errcode) {
	case ER_MEMORY_ISSUE:
		errcode = MEMCACHED_BIN_RES_ENOMEM;
		errstr  = NULL;
		rv = -1;
		break;
	case ER_TUPLE_NOT_FOUND:
		errcode = MEMCACHED_BIN_RES_KEY_ENOENT;
		errstr  = NULL;
		break;
	case ER_TUPLE_FOUND:
		errcode = MEMCACHED_BIN_RES_KEY_EEXISTS;
		errstr  = NULL;
		break;
	default:
		errcode = MEMCACHED_BIN_RES_SERVER_ERROR;
		snprintf(errfstr, 256, "SERVER ERROR '%s'", errstr);
		errstr = errfstr;
		rv = -1;
		break;
	}
	if (memcached_binary_error(con, errcode, errstr) == -1)
		rv = -1;
	return rv;
}
#endif

void
memcached_set_binary(struct memcached_connection *con)
{
	con->cb.parse_request   = memcached_binary_parse;
	con->cb.process_request = memcached_binary_process;
	con->cb.process_error   = memcached_binary_error;
	con->cb.write_answer    = memcached_binary_write;
	con->cb.process_stat    = memcached_binary_stat;
}
