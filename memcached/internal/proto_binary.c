#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>

#include <tarantool/module.h>
#include <msgpuck.h>

#include "error.h"
#include "utils.h"
#include "memcached.h"
#include "constants.h"
#include "memcached_layer.h"

#include "proto_binary.h"

#include <small/ibuf.h>
#include <small/obuf.h>

static inline int
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
		memcached_error_ENOMEM(to_alloc, "obuf");
		return -1;
	}
	size_t rv = obuf_dup(out, &hdro, sizeof(struct memcached_hdr));;
	if (ext) rv += obuf_dup(out, ext, ext_len);
	if (key) rv += obuf_dup(out, key, key_len);
	if (val) rv += obuf_dup(out, val, val_len);
	if (rv != to_alloc) {
		/* unreachable*/
		assert(0);
	}
	return 0;
}

static inline int
write_output_ok(struct memcached_connection *con, uint64_t cas,
		uint8_t ext_len, uint16_t key_len, uint32_t val_len,
		const char *ext, const char *key, const char *val
		)
{
	return memcached_binary_write(con, MEMCACHED_RES_OK,
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
		  memcached_bin_cmdname(h->cmd), h->opaque);
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
			  PRIu32, memcached_bin_cmdname(h->cmd),
			  b->key_len, b->key, b->val_len, b->val,
			  mp_bswap_u32(ext->flags), mp_bswap_u32(ext->expire));
		say_debug("opaque - %" PRIu32 ", cas - %" PRIu64,
			  h->opaque, h->cas);
	}

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, b->key, b->key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
	bool tuple_exists  = (tuple != NULL);
	bool tuple_expired = tuple_exists && is_expired_tuple(con->cfg, tuple);

	/* Check for key (non)existence for different commands */
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
	if (memcached_tuple_set(con, b->key, b->key_len, exptime, b->val,
				b->val_len, new_cas, ext->flags) == -1) {
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
		say_debug("%s '%.*s'", memcached_bin_cmdname(h->cmd),
			  b->key_len, b->key);
		say_debug("opaque - %" PRIu32, h->opaque);
	}

	if (h->cmd == MEMCACHED_BIN_CMD_GETQ ||
	    h->cmd == MEMCACHED_BIN_CMD_GETKQ)
		con->noreply = true;

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, b->key, b->key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
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
	if (memcached_binary_write(con, MEMCACHED_RES_OK, cas,
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
			  memcached_bin_cmdname(h->cmd), b->key_len,
			  b->key, h->opaque);

	con->cfg->stat.cmd_delete++;
	uint32_t len = mp_sizeof_array(1) +
		       mp_sizeof_str  (b->key_len);
	char *begin = (char *)box_txn_alloc(len);
	if (begin == NULL) {
		memcached_error_ENOMEM(len, "key");
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

	/* Get existence flags */
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
			  memcached_bin_cmdname(h->cmd), h->opaque);
	}

	const char *vers = PACKAGE_VERSION;
	int vlen = strlen(vers);
	if (memcached_binary_write(con, MEMCACHED_RES_OK, 0, 0, 0, vlen,
				   NULL, NULL, vers))
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
			  memcached_bin_cmdname(h->cmd),
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
			  memcached_bin_cmdname(h->cmd),
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
			  memcached_bin_cmdname(h->cmd),
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
		say_debug("%s '%.*s'", memcached_bin_cmdname(h->cmd),
			  b->key_len, b->key);
		say_debug("opaque - %" PRIu32 ", new expire - %" PRIu64,
			  mp_bswap_u32(h->opaque), exptime);
	}

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, b->key, b->key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
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
	if (memcached_tuple_set(con, kpos, klen, exptime, vpos, vlen,
				cas, flags) == -1) {
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
	if (memcached_binary_write(con, MEMCACHED_RES_OK, cas, elen, klen,
				   vlen, (const char *)epos, kpos, vpos) == -1)
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
			  memcached_bin_cmdname(h->cmd),
			  b->key_len, b->key, (uint64_t )mp_bswap_u64(ext->delta),
			  mp_bswap_u32(h->opaque));
		if (ext->expire == 0xFFFFFFFFLL)
			say_debug("default value is '%" PRIu64 "'",
				  (uint64_t )mp_bswap_u64(ext->initial));
	}

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, b->key, b->key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	uint64_t val = 0;
	uint64_t cas = con->cfg->cas++;
	const char *vpos = NULL;
	uint32_t    vlen = 0;
	char        strval[22]; uint8_t strvallen = 0;

	/* Get existence flags */
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
	strvallen = snprintf(strval, 22, "%" PRIu64, val);
	if (memcached_tuple_set(con, b->key, b->key_len, expire,
				(const char *)strval, strvallen,
				cas, 0) == -1) {
		box_txn_rollback();
		return -1;
	} else if (!con->noreply) {
		val = mp_bswap_u64(val);
		if (memcached_binary_write(con, MEMCACHED_RES_OK, cas, 0, 0,
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
			memcached_bin_cmdname(h->cmd),
			b->key_len, b->key, b->val_len, b->val,
			mp_bswap_u32(h->opaque));
	}

	con->cfg->stat.cmd_set++;

	box_tuple_t *tuple = NULL;
	if (memcached_tuple_get(con, b->key, b->key_len, &tuple) == -1) {
		box_txn_rollback();
		return -1;
	}

	/* Get existence flags */
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

	char *begin = (char *)box_txn_alloc(b->val_len + vlen);
	if (begin == NULL) {
		memcached_error_ENOMEM(b->val_len + vlen, "value");
		return -1;
	}

	uint64_t new_cas = con->cfg->cas++;

	if (h->cmd == MEMCACHED_BIN_CMD_PREPEND ||
	    h->cmd == MEMCACHED_BIN_CMD_PREPENDQ) {
		memcpy(begin, b->val, b->val_len);
		memcpy(begin + b->val_len, vpos, vlen);
	} else {
		memcpy(begin, vpos, vlen);
		memcpy(begin + vlen, b->val, b->val_len);
	}

	/* Tuple can't be NULL, because we already found this element */
	if (memcached_tuple_set(con, kpos, klen, exptime, begin,
				vlen + b->val_len, new_cas, flags) == -1) {
		box_txn_rollback();
		return -1;
	} else if (!con->noreply) {
		if (memcached_binary_write(con, MEMCACHED_RES_OK, new_cas,
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
			  memcached_bin_cmdname(h->cmd),
			  mp_bswap_u32(h->opaque));
	}

	con->close_connection = true;
	
	if (!con->noreply && write_output_ok_empty(con) != -1) {
		return -1;
	}
	return 0;
}

int
memcached_process_unsupported(struct memcached_connection *con)
{
	memcached_error_NOT_SUPPORTED(memcached_bin_cmdname(con->hdr->cmd));
	return -1;
}

int
memcached_process_unknown(struct memcached_connection *con)
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
	if (memcached_binary_write(con, 0, 0, 0, key_len, val_len, NULL,
				   key, val) == -1)
		return -1;
	return 0;
};

int
memcached_bin_process_stat(struct memcached_connection *con) {

	/* default declarations */
	struct memcached_body *b = &con->body;
	stat_func_t append = stat_append;

	/* ADD errstr for TODO */
	if (b->key_len == 0) {
		memcached_stat_all(con, append);
	} else if (b->key_len == 5  && !strncmp(b->key, "reset", 5)) {
		memcached_stat_reset(con, append);
/*	} else if (b->key_len == 6  && !strncmp(b->key, "detail", 6)) {
		memcached_error_NOT_SUPPORTED("stat detail");
		return -1;
	} else if (b->key_len == 11 && !strncmp(b->key, "detail dump", 11)) {
		memcached_error_NOT_SUPPORTED("stat detail dump");
		return -1;
	} else if (b->key_len == 9  && !strncmp(b->key, "detail on", 9)) {
		memcached_error_NOT_SUPPORTED("stat detail on");
		return -1;
	} else if (b->key_len == 10 && !strncmp(b->key, "detail off", 10)) {
		memcached_error_NOT_SUPPORTED("stat detail off");
		return -1;*/
	} else {
		char err[256] = {0};
		snprintf(err, 256, "stat %.*s", (int )b->key_len, b->key);
		memcached_error_NOT_SUPPORTED(err);
		return -1;
	}
	return 0;
}

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
	NULL
};

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
	if (memcached_binary_write(con, err, 0, 0, 0, len,
				   NULL, NULL, errstr) == -1)
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
	if (memcached_binary_write(con, 0, 0, 0, key_len,
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
}
