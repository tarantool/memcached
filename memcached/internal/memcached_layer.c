#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#include <tarantool.h>
#include <msgpuck/msgpuck.h>
#include <small/obuf.h>

#include "memcached.h"
#include "memcached_layer.h"

/* MEMCACHED_CONVERTION_FUNCTIONS */

#define xisspace(c) isspace((unsigned char)c)

static inline bool
safe_strtoull(const char *begin, const char *end, uint64_t *out)
{
	assert(out != NULL);
	errno = 0;
	*out = 0;
	char *endptr;
	unsigned long long ull = strtoull(begin, &endptr, 10);
	if ((errno == ERANGE) || (begin == endptr) || (endptr != end))
		return false;

	if (xisspace(*endptr) || (*endptr == '\0' && endptr != begin) ||
	    endptr == end) {
		if ((long long) ull < 0) {
			/* only check for negative signs in the uncommon
			 * case when the unsigned number is so big that
			 * it's negative as a signed number. */
			if (strchr(begin, '-') != NULL)
				return false;
		}
		*out = ull;
		return true;
	}
	return false;
}

/*
 * default exptime is 30*24*60*60 seconds
 * \* 1000000 to convert it to usec (need this precision)
 **/
#define MAX_EXPTIME (30*24*60*60LL)
#define INF_EXPTIME UINT64_MAX

static inline __attribute__((unused)) void
memcached_dump_hdr(struct memcached_hdr *hdr)
{
	if (!hdr) return;
	say_debug("memcached package");
	say_debug("magic:   0x%.2" PRIX8, hdr->magic);
	say_debug("cmd:     0x%.2" PRIX8, hdr->cmd);
	say_debug("key_len: %" PRIu16,    hdr->key_len);
	say_debug("ext_len: %" PRIu8,     hdr->ext_len);
	say_debug("tot_len: %" PRIu32,    hdr->tot_len);
	say_debug("opaque:  0x%" PRIX32,  mp_bswap_u32(hdr->opaque));
	say_debug("cas:     %" PRIu64,    hdr->cas);
}

static inline uint64_t
convert_exptime (uint64_t exptime)
{
	if (exptime == 0) return INF_EXPTIME; /* 0 means never expire */
	if (exptime <= MAX_EXPTIME)
		exptime = fiber_time64() + exptime * 1000000;
	else
		exptime = exptime * 1000000;
	return exptime;
}

static inline int
is_expired (uint64_t exptime, uint64_t time, uint64_t flush)
{
	(void )time;
	uint64_t curtime = fiber_time64();
	/* Expired by TTL or FLUSH */
	if (exptime <= curtime || (flush <= curtime && time <= flush))
		return 1;
	return 0;
}

int
is_expired_tuple(struct memcached_service *p, box_tuple_t *tuple)
{
	uint64_t flush = p->flush;
	const char *pos  = box_tuple_field(tuple, 1);
	uint64_t exptime = mp_decode_uint(&pos);
	uint64_t time    = mp_decode_uint(&pos);
	return is_expired(exptime, time, flush);
}

static inline int
write_output(struct memcached_connection *con, uint16_t err, uint64_t cas,
	     uint8_t ext_len, uint16_t key_len, uint32_t val_len,
	     const char *ext, const char *key, const char *val
	     )
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
		memcached_error_ENOMEM(to_alloc,"write_output","obuf");
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

static inline int
write_output_ok(struct memcached_connection *con, uint64_t cas,
		uint8_t ext_len, uint16_t key_len, uint32_t val_len,
		const char *ext, const char *key, const char *val
		)
{
	return write_output(con, MEMCACHED_BIN_RES_OK,
			    cas, ext_len, key_len,
			    val_len, ext, key, val);
}

static inline int
write_output_ok_empty(struct memcached_connection *con)
{
	return write_output_ok(con, 0, 0, 0, 0, NULL, NULL, NULL);
}

static inline int
write_output_ok_cas(struct memcached_connection *con, uint64_t cas)
{
	return write_output_ok(con, cas, 0, 0, 0, NULL, NULL, NULL);
}

static inline int
memcached_replace_tuple(struct memcached_connection *con,
			const char *kpos, uint32_t klen, uint64_t expire,
			const char *vpos, uint32_t vlen, uint64_t cas,
			uint32_t flags, uint32_t space_id)
{
	(void )con;
	uint64_t time = fiber_time64();
	uint32_t len = mp_sizeof_array(6)      +
		       mp_sizeof_str  (klen)   +
		       mp_sizeof_uint (expire) +
		       mp_sizeof_uint (time)   +
		       mp_sizeof_str  (vlen)   +
		       mp_sizeof_uint (cas)    +
		       mp_sizeof_uint (flags);
	char *begin  = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_replace_tuple","tuple");
		return -1;
	}
	char *end = mp_encode_array(begin, 6);
	      end = mp_encode_str  (end, kpos, klen);
	      end = mp_encode_uint (end, expire);
	      end = mp_encode_uint (end, time);
	      end = mp_encode_str  (end, vpos, vlen);
	      end = mp_encode_uint (end, cas);
	      end = mp_encode_uint (end, flags);
	assert(end <= begin + len);
	return box_replace(space_id, begin, end, NULL);
}

/**
 * Write error code message to output buffer
 *
 * \param[in] con    Connection object for writing output to
 * \param[in] err    Error code (status)
 * \param[in] errstr Eror string (if available) with description (may be NULL)
 *
 */
int memcached_bin_error(struct memcached_connection *con,
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
	if (write_output(con, err, 0, 0, 0, len, NULL, NULL, errstr) == -1)
		return -1;
	return 0;
}

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
	if (memcached_bin_error(con, errcode, errstr) == -1)
		rv = -1;
	return rv;
}

/**
 * ext/key/val - -1 - must not be presented
 *                0 - may be presented
 *                1 - must be presented
 **/
static inline int
memcached_package_verify(struct memcached_connection *con,
			 int ext, int key, int val, int ext_len)
{
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	assert(val != 0);
	assert((ext != -1 && ext_len != 0) || (ext == -1 && ext_len == 0));
	const char *section = NULL;

	/* Checking extra information */
	if ((ext == -1 && b->ext_len) || (ext == 1 && b->ext_len != ext_len) ||
	    (ext == 0 && (b->ext_len != ext_len && b->ext_len != 0))) {
		section = "ext";
		goto error;
	/* Checking key information */
	} else if ((key == -1 && b->key_len) || (key == 1  && !b->key)) {
		section = "key";
		goto error;
	/* Checking value information */
	} else if ((key == -1 && b->key_len) || (key == 1  && !b->key)) {
		section = "val";
		goto error;
	}

	return 0;
error:
	assert(section);
	say_error("problem while parsing package '%s' with opaque %" PRIu32,
		  memcached_get_command_name(h->cmd), h->opaque);
	say_error("erroneous '%s' section", section);
	con->close_connection = true;
	box_error_raise(ER_INVALID_MSGPACK, "Invalid arguments");
	return -1;
}

int
memcached_bin_process_set(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_ADDQ ||
	    h->cmd == MEMCACHED_BIN_CMD_SETQ ||
	    h->cmd == MEMCACHED_BIN_CMD_REPLACEQ)
		con->noreply = true;

	if (memcached_package_verify(con, 1, 1, 1,
			sizeof(struct memcached_set_ext)) == -1)
		return -1;
	
	int cmd = 0;
	switch (h->cmd) {
	case MEMCACHED_BIN_CMD_ADD:
	case MEMCACHED_BIN_CMD_ADDQ:
		cmd = MEMCACHED_SET_ADD;
		break;
	case MEMCACHED_BIN_CMD_SET:
	case MEMCACHED_BIN_CMD_SETQ:
		cmd = MEMCACHED_SET_SET;
		break;
	case MEMCACHED_BIN_CMD_REPLACE:
	case MEMCACHED_BIN_CMD_REPLACEQ:
		cmd = MEMCACHED_SET_REPLACE;
		break;
	default:
		assert(0);
	}
	if (h->cas != 0) cmd = MEMCACHED_SET_CAS;

	con->cfg->stat.cmd_set++;

	/* Prepare for run, extract cas and flags */
	struct memcached_set_ext *ext = (struct memcached_set_ext *)b->ext;
	ext->flags = mp_bswap_u32(ext->flags);
	uint64_t exptime = convert_exptime(mp_bswap_u32(ext->expire));

	if (con->cfg->verbosity > 1) {
		say_debug("%s '%.*s' '%.*s', flags - %" PRIu32 ", expire - %"
			  PRIu32, memcached_get_command_name(h->cmd),
			  b->key_len, b->key, b->val_len, b->val,
			  mp_bswap_u32(ext->flags), mp_bswap_u32(ext->expire));
		say_debug("opaque - %" PRIu32 ", cas - %" PRIu64,
			  h->opaque, h->cas);
	}

	/* Create key for getting previous tuple from space */
	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (b->key_len);
	char *begin  = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_set","key");
		return -1;
	}
	char *end = NULL;
	end = mp_encode_array(begin, 1);
	end = mp_encode_str  (end, b->key, b->key_len);
	assert(end <= begin + len);

	/* Get tuple from space */
	box_tuple_t *tuple = NULL;
	if (box_index_get(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	/* Check for key (non)existance for different commands */
	if (cmd == MEMCACHED_SET_REPLACE && (!tuple_exists || tuple_expired)) {
		memcached_error_KEY_ENOENT();
		return -1;
	} else if (cmd == MEMCACHED_SET_ADD && tuple_exists) {
		if (!tuple_expired) {
			memcached_error_KEY_EEXISTS();
			return -1;
		}
		con->cfg->stat.reclaimed++;
	} else if (cmd == MEMCACHED_SET_CAS) {
		if (!tuple_exists || tuple_expired) {
			con->cfg->stat.cas_misses++;
			memcached_error_KEY_ENOENT();
			return -1;
		}
		const char *pos   = box_tuple_field(tuple, 4);
		assert(mp_typeof(*pos) == MP_UINT);
		uint64_t cas_prev = mp_decode_uint(&pos);
		if (cas_prev != h->cas) {
			con->cfg->stat.cas_badval++;
			memcached_error_KEY_EEXISTS();
			return -1;
		}
		con->cfg->stat.cas_hits++;
	}

	uint64_t new_cas = con->cfg->cas++;
	if (memcached_replace_tuple(con, b->key, b->key_len, exptime, b->val,
				    b->val_len, new_cas, ext->flags,
				    con->cfg->space_id) == -1) {
		box_txn_rollback();
		return -1;
	} else if (!con->noreply && write_output_ok_cas(con, new_cas) == -1) {
		return -1;
	}
	return 0;
}

int
memcached_bin_process_get(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;
	
	con->cfg->stat.cmd_get++;

	if (memcached_package_verify(con, -1, 1, -1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s '%.*s'", memcached_get_command_name(h->cmd),
			  b->key_len, b->key);
		say_debug("opaque - %" PRIu32, h->opaque);
	}

	if (h->cmd == MEMCACHED_BIN_CMD_GETQ ||
	    h->cmd == MEMCACHED_BIN_CMD_GETKQ)
		con->noreply = true;

	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (b->key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_get","key");
		return -1;
	}
	char *end = NULL;
	      end = mp_encode_array(begin, 1);
	      end = mp_encode_str  (end, b->key, b->key_len);
	assert(end <= begin + len);
	box_tuple_t *tuple = NULL;
	if (box_index_get(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		con->cfg->stat.get_misses++;
		if (!con->noreply) {
			memcached_error_KEY_ENOENT();
			return -1;
		}
		return 0;
	}
	con->cfg->stat.get_hits++;
	struct memcached_get_ext ext;
	uint32_t vlen = 0, klen = 0;
	const char *pos  = box_tuple_field(tuple, 0);
	const char *kpos = mp_decode_str(&pos, &klen);
	mp_next(&pos); /* skip expiration time */
	mp_next(&pos); /* skip creation time */
	const char *vpos = mp_decode_str(&pos, &vlen);
	uint64_t cas     = mp_decode_uint(&pos);
	uint32_t flags   = mp_decode_uint(&pos);
	if (h->cmd == MEMCACHED_BIN_CMD_GET ||
	    h->cmd == MEMCACHED_BIN_CMD_GETQ) {
		kpos = NULL;
		klen = 0;
	}
	ext.flags = mp_bswap_u32(flags);
	if (write_output(con, MEMCACHED_BIN_RES_OK, cas,
			 sizeof(struct memcached_get_ext), klen, vlen,
			 (const char *)&ext, kpos, vpos) == -1)
		return -1;
	return 0;
}

int
memcached_bin_process_delete(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_DELETEQ)
		con->noreply = true;

	if (memcached_package_verify(con, -1, 1, -1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1)
		say_debug("%s '%.*s', opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd), b->key_len,
			  b->key, h->opaque);

	con->cfg->stat.cmd_delete++;
	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (b->key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_delete","key");
		return -1;
	}
	char *end   = mp_encode_array(begin, 1);
	      end   = mp_encode_str  (end, b->key, b->key_len);
	assert(end <= begin + len);
	box_tuple_t *tuple = NULL;
	if (box_delete(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	/* We didn't delete anything, or tuple is already expired */
	if (!tuple_exists || tuple_expired) {
		if (tuple_expired) con->cfg->stat.evictions++;
		con->cfg->stat.delete_misses++;
		memcached_error_KEY_ENOENT();
		return -1;
	}
	con->cfg->stat.delete_hits++;
	if (!con->noreply && write_output_ok_empty(con) == -1)
			return -1;
	return 0;
}

/* TODO: Move version creation to separate function */
int
memcached_bin_process_version(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;

	if (memcached_package_verify(con, -1, -1, -1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s, opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd), h->opaque);
	}

	const char *vers = PACKAGE_VERSION;
	int vlen = strlen(vers);
	if (write_output(con, MEMCACHED_BIN_RES_OK,
		         0, 0, 0, vlen, NULL, NULL, vers))
		return -1;
	return 0;
}

int
memcached_bin_process_noop(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;

	if (memcached_package_verify(con, -1, -1, -1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s, opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd),
			  mp_bswap_u32(h->opaque));
	}

	if (write_output_ok_empty(con) == -1)
		return -1;
	return 0;
}

int
memcached_bin_process_flush(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_FLUSHQ)
		con->noreply = true;

	if (memcached_package_verify(con, 0, -1, -1,
			sizeof(struct memcached_flush_ext)) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s, opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd),
			  mp_bswap_u32(h->opaque));
	}

	con->cfg->stat.cmd_flush++;
	struct memcached_flush_ext *ext = (struct memcached_flush_ext *)b->ext;
	uint64_t exptime = 0;
	con->cfg->flush = fiber_time64();
	if (ext != NULL) exptime = mp_bswap_u32(ext->expire);
	if (exptime > 0) con->cfg->flush = convert_exptime(exptime);
	if (!con->noreply && write_output_ok_empty(con) == -1) {
			return -1;
	}
	return 0;
}

int
memcached_bin_process_verbosity(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (memcached_package_verify(con, 0, -1, -1,
			sizeof(struct memcached_verb_ext)) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s, opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd),
			  mp_bswap_u32(h->opaque));
	}

	struct memcached_verb_ext *ext = (struct memcached_verb_ext *)b->ext;
	uint64_t verbosity = 0;
	if (ext != NULL) verbosity = mp_bswap_u32(ext->verbosity);
	if (verbosity <= 3) {
		con->cfg->verbosity = verbosity;
		if (!con->noreply && write_output_ok_empty(con) == -1) {
				return -1;
		}
	} else {
		memcached_error_EINVAL();
		return -1;
	}
	return 0;
}

int
memcached_bin_process_gat(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_GATQ ||
	    h->cmd == MEMCACHED_BIN_CMD_GATKQ)
		con->noreply = true;

	if (memcached_package_verify(con, 1, 1, -1,
			sizeof(struct memcached_touch_ext)) == -1)
		return -1;

	con->cfg->stat.cmd_touch++;
	struct memcached_touch_ext *ext = (struct memcached_touch_ext *)b->ext;
	uint64_t exptime = convert_exptime(mp_bswap_u32(ext->expire));

	if (con->cfg->verbosity > 1) {
		say_debug("%s '%.*s'", memcached_get_command_name(h->cmd),
			  b->key_len, b->key);
		say_debug("opaque - %" PRIu32 ", new expire - %" PRIu64,
			  mp_bswap_u32(h->opaque), exptime);
	}

	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str(b->key_len);
	char *begin  = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_gat","key");
		return -1;
	}
	char *end = mp_encode_array(begin, 2);
	end = mp_encode_str  (end, b->key, b->key_len);
	assert(end <= begin + len);

	box_tuple_t *tuple = NULL;
	if (box_index_get(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}
	
	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		con->cfg->stat.touch_misses++;
		if (!con->noreply) {
			memcached_error_KEY_ENOENT();
			return -1;
		}
		return 0;
	}
	con->cfg->stat.touch_hits++;

	uint32_t vlen = 0, klen = 0, elen = 0;
	const char *kpos = NULL, *vpos = NULL;
	uint32_t flags = 0;
	uint64_t cas = 0;
	struct memcached_get_ext *epos = NULL;
	const char *pos  = box_tuple_field(tuple, 0);
	kpos             = mp_decode_str(&pos, &klen);
	mp_next(&pos); /* skip expiration time */
	mp_next(&pos); /* skip creation time */
	vpos             = mp_decode_str(&pos, &vlen);
	cas              = mp_decode_uint(&pos);
	flags            = mp_decode_uint(&pos);
	epos             = (struct memcached_get_ext *)&flags;
	elen             = sizeof(struct memcached_get_ext);
	if (memcached_replace_tuple(con, kpos, klen, exptime, vpos, vlen,
				    cas, flags, con->cfg->space_id) == -1) {
		box_txn_rollback();
		return -1;
	}

	if (h->cmd >= MEMCACHED_BIN_CMD_GAT) {
		if (h->cmd == MEMCACHED_BIN_CMD_TOUCH) {
			vpos = NULL;
			vlen = 0;
		}
		if (h->cmd != MEMCACHED_BIN_CMD_GATK ||
		    h->cmd != MEMCACHED_BIN_CMD_GATKQ) {
			kpos = NULL;
			klen = 0;
		}
	}
	if (write_output(con, MEMCACHED_BIN_RES_OK, cas, elen, klen, vlen,
			 (const char *)epos, kpos, vpos) == -1)
		return -1;
	return 0;
}

int
memcached_bin_process_delta(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_INCRQ ||
	    h->cmd == MEMCACHED_BIN_CMD_DECRQ)
		con->noreply = true;

	if (memcached_package_verify(con, 1, 1, -1,
			sizeof(struct memcached_delta_ext)) == -1)
		return -1;

	struct memcached_delta_ext *ext = (struct memcached_delta_ext *)b->ext;
	uint64_t expire = convert_exptime(mp_bswap_u32(ext->expire));
	ext->delta      = mp_bswap_u64(ext->delta);
	ext->initial    = mp_bswap_u64(ext->initial);

	if (con->cfg->verbosity > 1) {
		say_debug("%s '%.*s' by %" PRIu64 ", opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd),
			  b->key_len, b->key, mp_bswap_u64(ext->delta),
			  mp_bswap_u32(h->opaque));
		if (ext->expire == 0xFFFFFFFFLL)
			say_debug("default value is '%" PRIu64 "'",
				  mp_bswap_u64(ext->initial));
	}

	/* Constructing key */
	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (b->key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_delta","key");
		return -1;
	}
	char *end   = mp_encode_array(begin, 1);
	      end   = mp_encode_str  (end, b->key, b->key_len);
	assert(end <= begin + len);
	box_tuple_t *tuple = NULL;
	uint64_t val = 0;
	uint64_t cas = con->cfg->cas++;
	const char *vpos = NULL;
	uint32_t    vlen = 0;
	char        strval[22]; uint8_t strvallen = 0;
	if (box_index_get(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		if (expire == 0xFFFFFFFFLL) {
			memcached_error_KEY_ENOENT();
			return -1;
		}
		if (!tuple_exists) con->cfg->stat.reclaimed++;
		val = ext->initial;
	} else {
		const char *pos = box_tuple_field(tuple, 3);
		vpos = mp_decode_str(&pos, &vlen);
		if (!safe_strtoull(vpos, vpos + vlen, &val)) {
			memcached_error_DELTA_BADVAL();
			return -1;
		}
		if (h->cmd == MEMCACHED_BIN_CMD_INCR ||
		    h->cmd == MEMCACHED_BIN_CMD_INCRQ) {
			val += ext->delta;
		} else if (ext->delta > val) {
			val = 0;
		} else {
			val -= ext->delta;
		}
	}

	/* Insert value */
	strvallen = snprintf(strval, 22, "%lu", val);
	if (memcached_replace_tuple(con, b->key, b->key_len, expire,
				    (const char *)strval, strvallen,
				    cas, 0, con->cfg->space_id) == -1) {
		box_txn_rollback();
		return -1;
	} else if (!con->noreply) {
		val = mp_bswap_u64(val);
		if (write_output(con, MEMCACHED_BIN_RES_OK, cas, 0, 0,
				 sizeof(val), NULL, NULL,
				 (const char *)&val) == -1)
			return -1;
	}
	return 0;
}

int
memcached_bin_process_pend(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;
	struct memcached_body *b = &con->body;

	if (h->cmd == MEMCACHED_BIN_CMD_APPENDQ ||
	    h->cmd == MEMCACHED_BIN_CMD_PREPENDQ)
		con->noreply = true;

	if (memcached_package_verify(con, -1, 1, 1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s to '%.*s' value '%.*s', opaque - %" PRIu32,
			memcached_get_command_name(h->cmd),
			b->key_len, b->key, b->val_len, b->val,
			mp_bswap_u32(h->opaque));
	}

	con->cfg->stat.cmd_set++;
	uint64_t new_cas = con->cfg->cas++;
	
	uint32_t len  = mp_sizeof_array (1) +
			mp_sizeof_str   (b->key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_pend","key");
		return -1;
	}
	char *end = mp_encode_array(begin, 1);
	      end = mp_encode_str  (end, b->key, b->key_len);

	box_tuple_t *tuple = NULL;
	if (box_index_get(con->cfg->space_id, 0, begin, end, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existance flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	if (!tuple_exists || tuple_expired) {
		memcached_error_KEY_ENOENT();
		return -1;
	}

	uint32_t vlen = 0, klen = 0;
	const char *kpos = NULL, *vpos = NULL;
	uint32_t flags = 0;
	uint64_t exptime = 0;
	const char *pos  = box_tuple_field(tuple, 0);
	kpos             = mp_decode_str(&pos, &klen);
	exptime          = mp_decode_uint(&pos);
	mp_next(&pos); /* skip creation time */
	vpos             = mp_decode_str(&pos, &vlen);
	mp_next(&pos); /* skip cas */
	flags            = mp_decode_uint(&pos);

	begin = (char *)box_txn_alloc(b->val_len + vlen);
	if (begin == NULL) {
		memcached_error_ENOMEM(len,"memcached_bin_process_pend","value");
		return -1;
	}
	if (h->cmd == MEMCACHED_BIN_CMD_PREPEND ||
	    h->cmd == MEMCACHED_BIN_CMD_PREPENDQ) {
		memcpy(begin, b->val, b->val_len);
		memcpy(begin + b->val_len, vpos, vlen);
	} else {
		memcpy(begin, vpos, vlen);
		memcpy(begin + vlen, b->val, b->val_len);
	}

	/* Tuple can't be NULL, because we already found this element */
	if (memcached_replace_tuple(con, kpos, klen, exptime, begin,
				    vlen + b->val_len, new_cas, flags,
				    con->cfg->space_id) == -1) {
		box_txn_rollback();
		return -1;
	} else if (!con->noreply) {
		if (write_output(con, MEMCACHED_BIN_RES_OK, new_cas,
				 0, 0, 0, NULL, NULL, NULL) == -1)
			return -1;
	}
	return 0;
}

int
memcached_bin_process_quit(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_hdr  *h = con->hdr;

	if (h->cmd == MEMCACHED_BIN_CMD_QUITQ)
		con->noreply = true;

	if (memcached_package_verify(con, -1, -1, -1, 0) == -1)
		return -1;

	if (con->cfg->verbosity > 1) {
		say_debug("%s, opaque - %" PRIu32,
			  memcached_get_command_name(h->cmd),
			  mp_bswap_u32(h->opaque));
	}

	con->close_connection = true;
	
	if (!con->noreply && write_output_ok_empty(con) != -1) {
		return -1;
	}
	return 0;
}

int
memcached_bin_process_unsupported(struct memcached_connection *con)
{
	memcached_error_NOT_SUPPORTED(memcached_get_command_name(con->hdr->cmd));
	return -1;
}

int
memcached_bin_process_unknown(struct memcached_connection *con)
{
	memcached_error_UNKNOWN_COMMAND(con->hdr->cmd);
	return -1;
}

static inline int
stat_append(struct memcached_connection *con, const char *key,
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
	if (write_output(con, 0, 0, 0, key_len,
			 val_len, NULL, key, val) == -1)
		return -1;
	return 0;
};

static inline void
memcached_bin_process_stat_reset(struct memcached_connection *con)
{
	memset(&con->cfg->stat, 0, sizeof(struct memcached_stat));
}

static inline void
memcached_bin_process_stat_all(struct memcached_connection *con)
{
	/* server specific data */
	stat_append(con, "pid", "%d", getpid());
/*	stat_append(con, "uptime", "%lf", tarantool_uptime()); */
	stat_append(con, "time", "%lf", fiber_time());
	stat_append(con, "version",      "Memcached (Tarantool "
					 PACKAGE_VERSION ")");
/*	stat_append(con, "libev")   */
	stat_append(con, "pointer_size",  "%d",  (int )(8 * sizeof(void *)));

	/* storage specific data */
	stat_append(con, "cmd_get",       "%lu", con->cfg->stat.cmd_get);
	stat_append(con, "get_hits",      "%lu", con->cfg->stat.get_hits);
	stat_append(con, "get_misses",    "%lu", con->cfg->stat.get_misses);
	stat_append(con, "cmd_set",       "%lu", con->cfg->stat.cmd_set);
	stat_append(con, "cas_hits",      "%lu", con->cfg->stat.cas_hits);
	stat_append(con, "cas_badval",    "%lu", con->cfg->stat.cas_badval);
	stat_append(con, "cas_misses",    "%lu", con->cfg->stat.cas_misses);
	stat_append(con, "cmd_delete",    "%lu", con->cfg->stat.cmd_delete);
	stat_append(con, "delete_hits",   "%lu", con->cfg->stat.delete_hits);
	stat_append(con, "delete_misses", "%lu", con->cfg->stat.delete_misses);
	stat_append(con, "cmd_incr",      "%lu", con->cfg->stat.cmd_incr);
	stat_append(con, "incr_hits",     "%lu", con->cfg->stat.incr_hits);
	stat_append(con, "incr_misses",   "%lu", con->cfg->stat.incr_misses);
	stat_append(con, "cmd_decr",      "%lu", con->cfg->stat.cmd_decr);
	stat_append(con, "decr_hits",     "%lu", con->cfg->stat.decr_hits);
	stat_append(con, "decr_misses",   "%lu", con->cfg->stat.decr_misses);
	stat_append(con, "cmd_flush",     "%lu", con->cfg->stat.cmd_flush);
	stat_append(con, "cmd_touch",     "%lu", con->cfg->stat.cmd_touch);
	stat_append(con, "touch_hits",    "%lu", con->cfg->stat.touch_hits);
	stat_append(con, "touch_misses",  "%lu", con->cfg->stat.touch_misses);
	stat_append(con, "evictions",     "%lu", con->cfg->stat.evictions);
	stat_append(con, "reclaimed",     "%lu", con->cfg->stat.reclaimed);
	stat_append(con, "auth_cmds",     "%lu", con->cfg->stat.auth_cmds);
	stat_append(con, "auth_errors",   "%lu", con->cfg->stat.auth_errors);
}

int
memcached_bin_process_stat(struct memcached_connection *con)
{
	/* default declarations */
	struct memcached_body *b = &con->body;

	/* ADD errstr for TODO */
	if (b->key_len == 0) {
		memcached_bin_process_stat_all(con);
	} else if (b->key_len == 5  && strcmp(b->key, "reset")) {
		memcached_bin_process_stat_reset(con);
	} else if (b->key_len == 6  && strcmp(b->key, "detail")) {
		memcached_error_NOT_SUPPORTED("stat detail");
		return -1;
	} else if (b->key_len == 11 && strcmp(b->key, "detail dump")) {;
		memcached_error_NOT_SUPPORTED("stat detail dump");
		return -1;
	} else if (b->key_len == 9  && strcmp(b->key, "detail on")) {;
		memcached_error_NOT_SUPPORTED("stat detail on");
		return -1;
	} else if (b->key_len == 10 && strcmp(b->key, "detail off")) {;
		memcached_error_NOT_SUPPORTED("stat detail off");
		return -1;
	} else {
		memcached_error_NOT_SUPPORTED("stat ---");
		return -1;
	}
	/* finish */
	stat_append(con, NULL, NULL);
	return 0;
}

const mc_process_func_t mc_handler[] = {
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
	memcached_bin_process_unknown,     /* RESERVED                   , 0x1f */
	memcached_bin_process_unsupported, /* MEMCACHED_._SASL_LIST_MECHS, 0x20 */
	memcached_bin_process_unsupported, /* MEMCACHED_._SASL_AUTH      , 0x21 */
	memcached_bin_process_unsupported, /* MEMCACHED_._SASL_STEP      , 0x22 */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GATK     , 0x23 */
	memcached_bin_process_gat,         /* MEMCACHED_BIN_CMD_GATKQ    , 0x24 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x25 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x26 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x27 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x28 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x29 */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2a */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2b */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2c */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2d */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2e */
	memcached_bin_process_unknown,     /* RESERVED                   , 0x2f */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RGET     , 0x30 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RSET     , 0x31 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RSETQ    , 0x32 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RAPPEND  , 0x33 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RAPPENDQ , 0x34 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RPREPEND , 0x35 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RPREPENDQ, 0x36 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RDELETE  , 0x37 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RDELETEQ , 0x38 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RINCR    , 0x39 */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RINCRQ   , 0x3a */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RDECR    , 0x3b */
	memcached_bin_process_unsupported, /* MEMCACHED_BIN_CMD_RDECRQ   , 0x3c */
};
