#include <stdio.h>

#include <tarantool.h>

#include "memcached.h"
#include "constants.h"
#include "memcached_layer.h"
#include "proto_text_parser.h"

#include <small/ibuf.h>
#include <small/obuf.h>

const mc_process_func_t memcached_txt_handler[] = {
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_SET,     0x00 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_ADD,     0x01 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_REPLACE, 0x02 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_APPEND,  0x03 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_PREPEND, 0x04 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_CAS,     0x05 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_GET,     0x06 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_GETS,    0x07 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_DELETE,  0x08 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_INCR,    0x09 */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_DECR,    0x0a */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_FLUSH,   0x0b */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_STATS,   0x0c */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_VERSION, 0x0d */
	memcached_process_unsupported, /* MEMCACHED_TXT_CMD_QUIT,    0x0e */
};


int
memcached_text_write(struct memcached_connection *con, uint16_t err,
		     uint64_t cas, uint8_t ext_len, uint16_t key_len,
		     uint32_t val_len, const char *ext,
		     const char *key, const char *val)
{
	(void )con;
	(void )err;
	(void )cas;
	(void )ext_len;
	(void )key_len;
	(void )val_len;
	(void )ext;
	(void )key;
	(void )val;
	return 0;
}


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
	con->noreply = false;
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
	(void )reqstart;
	int rv = memcached_text_parser(con, &reqstart, end);
	if (rv == 0) {
		con->len = reqstart - in->rpos;
	}
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
		errstr = memcached_get_result_title(err);
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
		   err == MEMCACHED_RES_E2BIG        ||
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

int
memcached_text_stat(struct memcached_connection *con, const char *key,
		    const char *valfmt, ...)
{
	struct obuf *out = con->out;
	if (!key) {
		obuf_dup(out, "END", 3);
	} else {
		char val_tmp[256] = {0};
		va_list va;
		va_start(va, valfmt);
		size_t val_len = vsnprintf(val_tmp, 256, valfmt, va);
		va_end(va);
		obuf_dup(out, "STAT ", 5);
		obuf_dup(out, key, strlen(key));
		obuf_dup(out, (const char *)val_tmp, val_len);
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
	con->cb.write_answer    = memcached_text_write;
	con->cb.process_stat    = memcached_text_stat;
}
