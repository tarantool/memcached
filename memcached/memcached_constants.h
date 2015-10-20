#ifndef   TARANTOOL_BOX_MEMCACHED_CONSTANTS_H_INCLUDED
#define   TARANTOOL_BOX_MEMCACHED_CONSTANTS_H_INCLUDED

/*
 * old memcached text protocol
 *
 * enum memcached_text_cmd {
 * 	MEMCACHED_TXT_CMD_SET     = 0x01,
 * 	MEMCACHED_TXT_CMD_ADD     = 0x02,
 * 	MEMCACHED_TXT_CMD_REPLACE = 0x03,
 * 	MEMCACHED_TXT_CMD_APPEND  = 0x04,
 * 	MEMCACHED_TXT_CMD_PREPEND = 0x05,
 * 	MEMCACHED_TXT_CMD_CAS     = 0x06,
 * 	MEMCACHED_TXT_CMD_GET     = 0x07,
 * 	MEMCACHED_TXT_CMD_GETS    = 0x08,
 * 	MEMCACHED_TXT_CMD_DELETE  = 0x09,
 * 	MEMCACHED_TXT_CMD_INCR    = 0x0a,
 * 	MEMCACHED_TXT_CMD_DECR    = 0x0b,
 * 	MEMCACHED_TXT_CMD_FLUSH   = 0x0c,
 * 	MEMCACHED_TXT_CMD_STATS   = 0x0d,
 * 	MEMCACHED_TXT_CMD_VERSION = 0x0e,
 * 	MEMCACHED_TXT_CMD_QUIT    = 0x0f,
 * };
 *
 * struct memcached_text_request {
 * 	enum memcached_op op;
 * 	const char *key;
 * 	size_t      key_len;
 * 	uint32_t    key_count;
 * 	const char *data;
 * 	size_t      data_len;
 * 	uint64_t    flags;
 * 	uint64_t    bytes;
 * 	uint64_t    cas;
 * 	uint64_t    exptime;
 * 	uint64_t    inc_val;
 * 	bool        noreply;
 * };
 */

enum memcached_binary_type {
	MEMCACHED_BIN_REQUEST  = 0x80,
	MEMCACHED_BIN_RESPONSE = 0x81
};

enum memcached_binary_response {
	MEMCACHED_BIN_RES_OK              = 0x00,
	MEMCACHED_BIN_RES_KEY_ENOENT      = 0x01,
	MEMCACHED_BIN_RES_KEY_EEXISTS     = 0x02,
	MEMCACHED_BIN_RES_E2BIG           = 0x03,
	MEMCACHED_BIN_RES_EINVAL          = 0x04,
	MEMCACHED_BIN_RES_NOT_STORED      = 0x05,
	MEMCACHED_BIN_RES_DELTA_BADVAL    = 0x06,
	MEMCACHED_BIN_RES_VBUCKET_BADVAL  = 0x07, /* No in memc */
	MEMCACHED_BIN_RES_AUTH_ERROR      = 0x20, /* OR 0x08 in revmapped */
	MEMCACHED_BIN_RES_AUTH_CONTINUE   = 0x21, /* OR 0x09 in revmapped */
	MEMCACHED_BIN_RES_UNKNOWN_COMMAND = 0x81,
	MEMCACHED_BIN_RES_ENOMEM          = 0x82,
	MEMCACHED_BIN_RES_NOT_SUPPORTED   = 0x83,
	MEMCACHED_BIN_RES_SERVER_ERROR    = 0x84, /* No in memc */
	MEMCACHED_BIN_RES_EBUSY           = 0x85, /* No in memc */
	MEMCACHED_BIN_RES_EAGAIN          = 0x86  /* No in memc */
};

enum memcached_binary_cmd {
	MEMCACHED_BIN_CMD_GET      = 0x00,
	MEMCACHED_BIN_CMD_SET      = 0x01,
	MEMCACHED_BIN_CMD_ADD      = 0x02,
	MEMCACHED_BIN_CMD_REPLACE  = 0x03,
	MEMCACHED_BIN_CMD_DELETE   = 0x04,
	MEMCACHED_BIN_CMD_INCR     = 0x05,
	MEMCACHED_BIN_CMD_DECR     = 0x06,
	MEMCACHED_BIN_CMD_QUIT     = 0x07,
	MEMCACHED_BIN_CMD_FLUSH    = 0x08,
	MEMCACHED_BIN_CMD_GETQ     = 0x09,
	MEMCACHED_BIN_CMD_NOOP     = 0x0a,
	MEMCACHED_BIN_CMD_VERSION  = 0x0b,
	MEMCACHED_BIN_CMD_GETK     = 0x0c,
	MEMCACHED_BIN_CMD_GETKQ    = 0x0d,
	MEMCACHED_BIN_CMD_APPEND   = 0x0e,
	MEMCACHED_BIN_CMD_PREPEND  = 0x0f,
	MEMCACHED_BIN_CMD_STAT     = 0x10,
	MEMCACHED_BIN_CMD_SETQ     = 0x11,
	MEMCACHED_BIN_CMD_ADDQ     = 0x12,
	MEMCACHED_BIN_CMD_REPLACEQ = 0x13,
	MEMCACHED_BIN_CMD_DELETEQ  = 0x14,
	MEMCACHED_BIN_CMD_INCRQ    = 0x15,
	MEMCACHED_BIN_CMD_DECRQ    = 0x16,
	MEMCACHED_BIN_CMD_QUITQ    = 0x17,
	MEMCACHED_BIN_CMD_FLUSHQ   = 0x18,
	MEMCACHED_BIN_CMD_APPENDQ  = 0x19,
	MEMCACHED_BIN_CMD_PREPENDQ = 0x1a,
	MEMCACHED_BIN_CMD_VERBOSITY= 0x1b,
	MEMCACHED_BIN_CMD_TOUCH    = 0x1c,
	MEMCACHED_BIN_CMD_GAT      = 0x1d,
	MEMCACHED_BIN_CMD_GATQ     = 0x1e,

	MEMCACHED_BIN_CMD_SASL_LIST_MECHS = 0x20,
	MEMCACHED_BIN_CMD_SASL_AUTH       = 0x21,
	MEMCACHED_BIN_CMD_SASL_STEP       = 0x22,

	MEMCACHED_BIN_CMD_GATK     = 0x23,
	MEMCACHED_BIN_CMD_GATKQ    = 0x24,

	/* These commands are used for range operations and exist within
	 * this header for use in other projects.  Range operations are
	 * not expected to be implemented in the memcached server itself.
	 */
	MEMCACHED_BIN_CMD_RGET      = 0x30,
	MEMCACHED_BIN_CMD_RSET      = 0x31,
	MEMCACHED_BIN_CMD_RSETQ     = 0x32,
	MEMCACHED_BIN_CMD_RAPPEND   = 0x33,
	MEMCACHED_BIN_CMD_RAPPENDQ  = 0x34,
	MEMCACHED_BIN_CMD_RPREPEND  = 0x35,
	MEMCACHED_BIN_CMD_RPREPENDQ = 0x36,
	MEMCACHED_BIN_CMD_RDELETE   = 0x37,
	MEMCACHED_BIN_CMD_RDELETEQ  = 0x38,
	MEMCACHED_BIN_CMD_RINCR     = 0x39,
	MEMCACHED_BIN_CMD_RINCRQ    = 0x3a,
	MEMCACHED_BIN_CMD_RDECR     = 0x3b,
	MEMCACHED_BIN_CMD_RDECRQ    = 0x3c,
	/* End Range operations */
	MEMCACHED_BIN_CMD_MAX
};

extern const char *memcached_binary_cmd_name[];

static inline const char *
memcached_get_command_name(uint32_t op) {
	if (op >= MEMCACHED_BIN_CMD_MAX)
		return "UNKNOWN";
	return memcached_binary_cmd_name[op];
}

enum memcached_binary_datatypes {
	MEMCACHED_BIN_RAW_BYTES = 0x00
};

struct __attribute__((__packed__ )) memcached_hdr {
	uint8_t  magic;
	uint8_t  cmd;
	uint16_t key_len;

	uint8_t  ext_len;
	uint8_t  data_type;
	union {
		uint16_t reserved;
		uint16_t status;
	};

	uint32_t tot_len;
	uint32_t opaque;
	uint64_t cas;
};

struct __attribute__((__packed__ )) memcached_get_ext {
	uint32_t flags;
};

struct __attribute__((__packed__ )) memcached_set_ext {
	uint32_t flags;
	uint32_t expire;
};

struct __attribute__((__packed__ )) memcached_delta_ext {
	uint64_t delta;
	uint64_t initial;
	uint32_t expire;
};

struct __attribute__((__packed__ )) memcached_flush_ext {
	uint32_t expire;
};

struct __attribute__((__packed__ )) memcached_verb_ext {
	uint32_t verbosity;
};

struct __attribute__((__packed__ )) memcached_touch_ext {
	uint32_t expire;
};

struct memcached_body {
	uint8_t     ext_len;
	uint16_t    key_len;
	uint32_t    val_len;
	const void *ext;
	const char *key;
	const char *val;
};

#endif /* TARANTOOL_BOX_MEMCACHED_CONSTANTS_H_INCLUDED */
