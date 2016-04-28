#ifndef   CONSTANTS_H_INCLUDED
#define   CONSTANTS_H_INCLUDED

#include "utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ENUM0_MEMBER_TXT(s, ...) MEMCACHED_TXT_CMD_##s,
#define ENUM0_MEMBER_BIN(s, ...) MEMCACHED_BIN_CMD_##s,

#define ENUM0_TXT(enum_name, enum_members) enum enum_name { enum_members(ENUM0_MEMBER_TXT) enum_name##_MAX }
#define ENUM0_BIN(enum_name, enum_members) enum enum_name { enum_members(ENUM0_MEMBER_BIN) enum_name##_MAX }

struct memcached_response_record {
	const char *name;
	const char *description;
	uint8_t     verbosity;
};

#define TEXT_COMMANDS(_)		\
		/* 0X00 */ _(UNKNOWN)	\
		/* 0X01 */ _(SET)	\
		/* 0X02 */ _(ADD)	\
		/* 0X03 */ _(REPLACE)	\
		/* 0X04 */ _(APPEND)	\
		/* 0X05 */ _(PREPEND)	\
		/* 0X06 */ _(CAS)	\
		/* 0X07 */ _(GET)	\
		/* 0X08 */ _(GETS)	\
		/* 0X09 */ _(DELETE)	\
		/* 0X0a */ _(INCR)	\
		/* 0X0b */ _(DECR)	\
		/* 0X0c */ _(FLUSH)	\
		/* 0X0d */ _(STATS)	\
		/* 0X0e */ _(VERSION)	\
		/* 0X0f */ _(QUIT)	\
		/* 0X10 */ _(VERBOSITY)

#define BINARY_COMMANDS(_)				\
		/* 0x00 */ _(GET)			\
		/* 0x01 */ _(SET)			\
		/* 0x02 */ _(ADD)			\
		/* 0x03 */ _(REPLACE)			\
		/* 0x04 */ _(DELETE)			\
		/* 0x05 */ _(INCR)			\
		/* 0x06 */ _(DECR)			\
		/* 0x07 */ _(QUIT)			\
		/* 0x08 */ _(FLUSH)			\
		/* 0x09 */ _(GETQ)			\
		/* 0x0a */ _(NOOP)			\
		/* 0x0b */ _(VERSION)			\
		/* 0x0c */ _(GETK)			\
		/* 0x0d */ _(GETKQ)			\
		/* 0x0e */ _(APPEND)			\
		/* 0x0f */ _(PREPEND)			\
		/* 0x10 */ _(STAT)			\
		/* 0x11 */ _(SETQ)			\
		/* 0x12 */ _(ADDQ)			\
		/* 0x13 */ _(REPLACEQ)			\
		/* 0x14 */ _(DELETEQ)			\
		/* 0x15 */ _(INCRQ)			\
		/* 0x16 */ _(DECRQ)			\
		/* 0x17 */ _(QUITQ)			\
		/* 0x18 */ _(FLUSHQ)			\
		/* 0x19 */ _(APPENDQ)			\
		/* 0x1a */ _(PREPENDQ)			\
		/* 0x1b */ _(VERBOSITY)			\
		/* 0x1c */ _(TOUCH)			\
		/* 0x1d */ _(GAT)			\
		/* 0x1e */ _(GATQ)			\
		/*<< 0x1f */ _(RESERVED_1f)		\
		/* 0x20 */ _(SASL_LIST_MECHS)		\
		/* 0x21 */ _(SASL_AUTH)			\
		/* 0x22 */ _(SASL_STEP)			\
		/* 0x23 */ _(GATK)			\
		/* 0x24 */ _(GATKQ)			\
		/*<< Reserved from 0x24 to 0x2f >>*/	\
		/*<< 0x24 */ _(RESERVED_24)		\
		/*<< 0x25 */ _(RESERVED_25)		\
		/*<< 0x26 */ _(RESERVED_26)		\
		/*<< 0x27 */ _(RESERVED_27)		\
		/*<< 0x28 */ _(RESERVED_28)		\
		/*<< 0x29 */ _(RESERVED_29)		\
		/*<< 0x2a */ _(RESERVED_2a)		\
		/*<< 0x2b */ _(RESERVED_2b)		\
		/*<< 0x2c */ _(RESERVED_2c)		\
		/*<< 0x2d */ _(RESERVED_2d)		\
		/*<< 0x2e */ _(RESERVED_2e)		\
		/*<< 0x2f */ _(RESERVED_2f)		\
		/*<< Reserved from 0x24 to 0x2f >>*/	\
		/* 0x30 */ _(RGET)			\
		/* 0x31 */ _(RSET)			\
		/* 0x32 */ _(RSETQ)			\
		/* 0x33 */ _(RAPPEND)			\
		/* 0x34 */ _(RAPPENDQ)			\
		/* 0x35 */ _(RPREPEND)			\
		/* 0x36 */ _(RPREPENDQ)			\
		/* 0x37 */ _(RDELETE)			\
		/* 0x38 */ _(RDELETEQ)			\
		/* 0x39 */ _(RINCR)			\
		/* 0x3a */ _(RINCRQ)			\
		/* 0x3b */ _(RDECR)			\
		/* 0x3c */ _(RDECRQ)

#define RESPONSE_CODES(_)										\
		/* 0x00 */ _(MEMCACHED_RES_OK,			0, "Not an error")			\
		/* 0x01 */ _(MEMCACHED_RES_KEY_ENOENT,		2, "Not found")				\
		/* 0x02 */ _(MEMCACHED_RES_KEY_EEXISTS,		2, "Data exists for key")		\
		/* 0x03 */ _(MEMCACHED_RES_E2BIG,		1, "Object too large for cache or OOM")	\
		/* 0x04 */ _(MEMCACHED_RES_EINVAL,		1, "Invalid arguments")			\
		/* 0x05 */ _(MEMCACHED_RES_NOT_STORED,		2, "Not stored")			\
		/* 0x06 */ _(MEMCACHED_RES_DELTA_BADVAL,	1, "Can't increment or decrement "	\
								   "non-numeric value")			\
		/* Not presented in vanilla memcached */						\
		/* 0x07 */ _(MEMCACHED_RES_VBUCKET_BADVAL,	1, "Bad vbucket value")			\
		/*<< Reserved from 0x08 to 0x1f >>*/							\
		/*<< 0x08 */ _(MEMCACHED_RES_RESERVED_08,	0, "Reserved 0x08")			\
		/*<< 0x09 */ _(MEMCACHED_RES_RESERVED_09,	0, "Reserved 0x09")			\
		/*<< 0x0a */ _(MEMCACHED_RES_RESERVED_0a,	0, "Reserved 0x0a")			\
		/*<< 0x0b */ _(MEMCACHED_RES_RESERVED_0b,	0, "Reserved 0x0b")			\
		/*<< 0x0c */ _(MEMCACHED_RES_RESERVED_0c,	0, "Reserved 0x0c")			\
		/*<< 0x0d */ _(MEMCACHED_RES_RESERVED_0d,	0, "Reserved 0x0d")			\
		/*<< 0x0e */ _(MEMCACHED_RES_RESERVED_0e,	0, "Reserved 0x0e")			\
		/*<< 0x0f */ _(MEMCACHED_RES_RESERVED_0f,	0, "Reserved 0x0f")			\
		/*<< 0x10 */ _(MEMCACHED_RES_RESERVED_10,	0, "Reserved 0x10")			\
		/*<< 0x11 */ _(MEMCACHED_RES_RESERVED_11,	0, "Reserved 0x11")			\
		/*<< 0x12 */ _(MEMCACHED_RES_RESERVED_12,	0, "Reserved 0x12")			\
		/*<< 0x13 */ _(MEMCACHED_RES_RESERVED_13,	0, "Reserved 0x13")			\
		/*<< 0x14 */ _(MEMCACHED_RES_RESERVED_14,	0, "Reserved 0x14")			\
		/*<< 0x15 */ _(MEMCACHED_RES_RESERVED_15,	0, "Reserved 0x15")			\
		/*<< 0x16 */ _(MEMCACHED_RES_RESERVED_16,	0, "Reserved 0x16")			\
		/*<< 0x17 */ _(MEMCACHED_RES_RESERVED_17,	0, "Reserved 0x17")			\
		/*<< 0x18 */ _(MEMCACHED_RES_RESERVED_18,	0, "Reserved 0x18")			\
		/*<< 0x19 */ _(MEMCACHED_RES_RESERVED_19,	0, "Reserved 0x19")			\
		/*<< 0x1a */ _(MEMCACHED_RES_RESERVED_1a,	0, "Reserved 0x1a")			\
		/*<< 0x1b */ _(MEMCACHED_RES_RESERVED_1b,	0, "Reserved 0x1b")			\
		/*<< 0x1c */ _(MEMCACHED_RES_RESERVED_1c,	0, "Reserved 0x1c")			\
		/*<< 0x1d */ _(MEMCACHED_RES_RESERVED_1d,	0, "Reserved 0x1d")			\
		/*<< 0x1e */ _(MEMCACHED_RES_RESERVED_1e,	0, "Reserved 0x1e")			\
		/*<< 0x1f */ _(MEMCACHED_RES_RESERVED_1f,	0, "Reserved 0x1f")			\
		/*<< Reserved from 0x08 to 0x1f >>*/							\
		/* In revmapped specification it's 0x08*/						\
		/* 0x20 */ _(MEMCACHED_RES_AUTH_ERROR,		2, "Authentication error")		\
		/* In revmapped specification it's 0x09*/						\
		/* 0x21 */ _(MEMCACHED_RES_AUTH_CONTINUE,	3, "Authentication proceed")		\
		/*<< Reserved from 0x22 to 0x80 >>*/							\
		/*<< 0x22 */ _(MEMCACHED_RES_RESERVED_22,	0, "Reserved 0x22")			\
		/*<< 0x23 */ _(MEMCACHED_RES_RESERVED_23,	0, "Reserved 0x23")			\
		/*<< 0x24 */ _(MEMCACHED_RES_RESERVED_24,	0, "Reserved 0x24")			\
		/*<< 0x25 */ _(MEMCACHED_RES_RESERVED_25,	0, "Reserved 0x25")			\
		/*<< 0x26 */ _(MEMCACHED_RES_RESERVED_26,	0, "Reserved 0x26")			\
		/*<< 0x27 */ _(MEMCACHED_RES_RESERVED_27,	0, "Reserved 0x27")			\
		/*<< 0x28 */ _(MEMCACHED_RES_RESERVED_28,	0, "Reserved 0x28")			\
		/*<< 0x29 */ _(MEMCACHED_RES_RESERVED_29,	0, "Reserved 0x29")			\
		/*<< 0x2a */ _(MEMCACHED_RES_RESERVED_2a,	0, "Reserved 0x2a")			\
		/*<< 0x2b */ _(MEMCACHED_RES_RESERVED_2b,	0, "Reserved 0x2b")			\
		/*<< 0x2c */ _(MEMCACHED_RES_RESERVED_2c,	0, "Reserved 0x2c")			\
		/*<< 0x2d */ _(MEMCACHED_RES_RESERVED_2d,	0, "Reserved 0x2d")			\
		/*<< 0x2e */ _(MEMCACHED_RES_RESERVED_2e,	0, "Reserved 0x2e")			\
		/*<< 0x2f */ _(MEMCACHED_RES_RESERVED_2f,	0, "Reserved 0x2f")			\
		/*<< 0x30 */ _(MEMCACHED_RES_RESERVED_30,	0, "Reserved 0x30")			\
		/*<< 0x31 */ _(MEMCACHED_RES_RESERVED_31,	0, "Reserved 0x31")			\
		/*<< 0x32 */ _(MEMCACHED_RES_RESERVED_32,	0, "Reserved 0x32")			\
		/*<< 0x33 */ _(MEMCACHED_RES_RESERVED_33,	0, "Reserved 0x33")			\
		/*<< 0x34 */ _(MEMCACHED_RES_RESERVED_34,	0, "Reserved 0x34")			\
		/*<< 0x35 */ _(MEMCACHED_RES_RESERVED_35,	0, "Reserved 0x35")			\
		/*<< 0x36 */ _(MEMCACHED_RES_RESERVED_36,	0, "Reserved 0x36")			\
		/*<< 0x37 */ _(MEMCACHED_RES_RESERVED_37,	0, "Reserved 0x37")			\
		/*<< 0x38 */ _(MEMCACHED_RES_RESERVED_38,	0, "Reserved 0x38")			\
		/*<< 0x39 */ _(MEMCACHED_RES_RESERVED_39,	0, "Reserved 0x39")			\
		/*<< 0x3a */ _(MEMCACHED_RES_RESERVED_3a,	0, "Reserved 0x3a")			\
		/*<< 0x3b */ _(MEMCACHED_RES_RESERVED_3b,	0, "Reserved 0x3b")			\
		/*<< 0x3c */ _(MEMCACHED_RES_RESERVED_3c,	0, "Reserved 0x3c")			\
		/*<< 0x3d */ _(MEMCACHED_RES_RESERVED_3d,	0, "Reserved 0x3d")			\
		/*<< 0x3e */ _(MEMCACHED_RES_RESERVED_3e,	0, "Reserved 0x3e")			\
		/*<< 0x3f */ _(MEMCACHED_RES_RESERVED_3f,	0, "Reserved 0x3f")			\
		/*<< 0x40 */ _(MEMCACHED_RES_RESERVED_40,	0, "Reserved 0x40")			\
		/*<< 0x41 */ _(MEMCACHED_RES_RESERVED_41,	0, "Reserved 0x41")			\
		/*<< 0x42 */ _(MEMCACHED_RES_RESERVED_42,	0, "Reserved 0x42")			\
		/*<< 0x43 */ _(MEMCACHED_RES_RESERVED_43,	0, "Reserved 0x43")			\
		/*<< 0x44 */ _(MEMCACHED_RES_RESERVED_44,	0, "Reserved 0x44")			\
		/*<< 0x45 */ _(MEMCACHED_RES_RESERVED_45,	0, "Reserved 0x45")			\
		/*<< 0x46 */ _(MEMCACHED_RES_RESERVED_46,	0, "Reserved 0x46")			\
		/*<< 0x47 */ _(MEMCACHED_RES_RESERVED_47,	0, "Reserved 0x47")			\
		/*<< 0x48 */ _(MEMCACHED_RES_RESERVED_48,	0, "Reserved 0x48")			\
		/*<< 0x49 */ _(MEMCACHED_RES_RESERVED_49,	0, "Reserved 0x49")			\
		/*<< 0x4a */ _(MEMCACHED_RES_RESERVED_4a,	0, "Reserved 0x4a")			\
		/*<< 0x4b */ _(MEMCACHED_RES_RESERVED_4b,	0, "Reserved 0x4b")			\
		/*<< 0x4c */ _(MEMCACHED_RES_RESERVED_4c,	0, "Reserved 0x4c")			\
		/*<< 0x4d */ _(MEMCACHED_RES_RESERVED_4d,	0, "Reserved 0x4d")			\
		/*<< 0x4e */ _(MEMCACHED_RES_RESERVED_4e,	0, "Reserved 0x4e")			\
		/*<< 0x4f */ _(MEMCACHED_RES_RESERVED_4f,	0, "Reserved 0x4f")			\
		/*<< 0x50 */ _(MEMCACHED_RES_RESERVED_50,	0, "Reserved 0x50")			\
		/*<< 0x51 */ _(MEMCACHED_RES_RESERVED_51,	0, "Reserved 0x51")			\
		/*<< 0x52 */ _(MEMCACHED_RES_RESERVED_52,	0, "Reserved 0x52")			\
		/*<< 0x53 */ _(MEMCACHED_RES_RESERVED_53,	0, "Reserved 0x53")			\
		/*<< 0x54 */ _(MEMCACHED_RES_RESERVED_54,	0, "Reserved 0x54")			\
		/*<< 0x55 */ _(MEMCACHED_RES_RESERVED_55,	0, "Reserved 0x55")			\
		/*<< 0x56 */ _(MEMCACHED_RES_RESERVED_56,	0, "Reserved 0x56")			\
		/*<< 0x57 */ _(MEMCACHED_RES_RESERVED_57,	0, "Reserved 0x57")			\
		/*<< 0x58 */ _(MEMCACHED_RES_RESERVED_58,	0, "Reserved 0x58")			\
		/*<< 0x59 */ _(MEMCACHED_RES_RESERVED_59,	0, "Reserved 0x59")			\
		/*<< 0x5a */ _(MEMCACHED_RES_RESERVED_5a,	0, "Reserved 0x5a")			\
		/*<< 0x5b */ _(MEMCACHED_RES_RESERVED_5b,	0, "Reserved 0x5b")			\
		/*<< 0x5c */ _(MEMCACHED_RES_RESERVED_5c,	0, "Reserved 0x5c")			\
		/*<< 0x5d */ _(MEMCACHED_RES_RESERVED_5d,	0, "Reserved 0x5d")			\
		/*<< 0x5e */ _(MEMCACHED_RES_RESERVED_5e,	0, "Reserved 0x5e")			\
		/*<< 0x5f */ _(MEMCACHED_RES_RESERVED_5f,	0, "Reserved 0x5f")			\
		/*<< 0x60 */ _(MEMCACHED_RES_RESERVED_60,	0, "Reserved 0x60")			\
		/*<< 0x61 */ _(MEMCACHED_RES_RESERVED_61,	0, "Reserved 0x61")			\
		/*<< 0x62 */ _(MEMCACHED_RES_RESERVED_62,	0, "Reserved 0x62")			\
		/*<< 0x63 */ _(MEMCACHED_RES_RESERVED_63,	0, "Reserved 0x63")			\
		/*<< 0x64 */ _(MEMCACHED_RES_RESERVED_64,	0, "Reserved 0x64")			\
		/*<< 0x65 */ _(MEMCACHED_RES_RESERVED_65,	0, "Reserved 0x65")			\
		/*<< 0x66 */ _(MEMCACHED_RES_RESERVED_66,	0, "Reserved 0x66")			\
		/*<< 0x67 */ _(MEMCACHED_RES_RESERVED_67,	0, "Reserved 0x67")			\
		/*<< 0x68 */ _(MEMCACHED_RES_RESERVED_68,	0, "Reserved 0x68")			\
		/*<< 0x69 */ _(MEMCACHED_RES_RESERVED_69,	0, "Reserved 0x69")			\
		/*<< 0x6a */ _(MEMCACHED_RES_RESERVED_6a,	0, "Reserved 0x6a")			\
		/*<< 0x6b */ _(MEMCACHED_RES_RESERVED_6b,	0, "Reserved 0x6b")			\
		/*<< 0x6c */ _(MEMCACHED_RES_RESERVED_6c,	0, "Reserved 0x6c")			\
		/*<< 0x6d */ _(MEMCACHED_RES_RESERVED_6d,	0, "Reserved 0x6d")			\
		/*<< 0x6e */ _(MEMCACHED_RES_RESERVED_6e,	0, "Reserved 0x6e")			\
		/*<< 0x6f */ _(MEMCACHED_RES_RESERVED_6f,	0, "Reserved 0x6f")			\
		/*<< 0x70 */ _(MEMCACHED_RES_RESERVED_70,	0, "Reserved 0x70")			\
		/*<< 0x71 */ _(MEMCACHED_RES_RESERVED_71,	0, "Reserved 0x71")			\
		/*<< 0x72 */ _(MEMCACHED_RES_RESERVED_72,	0, "Reserved 0x72")			\
		/*<< 0x73 */ _(MEMCACHED_RES_RESERVED_73,	0, "Reserved 0x73")			\
		/*<< 0x74 */ _(MEMCACHED_RES_RESERVED_74,	0, "Reserved 0x74")			\
		/*<< 0x75 */ _(MEMCACHED_RES_RESERVED_75,	0, "Reserved 0x75")			\
		/*<< 0x76 */ _(MEMCACHED_RES_RESERVED_76,	0, "Reserved 0x76")			\
		/*<< 0x77 */ _(MEMCACHED_RES_RESERVED_77,	0, "Reserved 0x77")			\
		/*<< 0x78 */ _(MEMCACHED_RES_RESERVED_78,	0, "Reserved 0x78")			\
		/*<< 0x79 */ _(MEMCACHED_RES_RESERVED_79,	0, "Reserved 0x79")			\
		/*<< 0x7a */ _(MEMCACHED_RES_RESERVED_7a,	0, "Reserved 0x7a")			\
		/*<< 0x7b */ _(MEMCACHED_RES_RESERVED_7b,	0, "Reserved 0x7b")			\
		/*<< 0x7c */ _(MEMCACHED_RES_RESERVED_7c,	0, "Reserved 0x7c")			\
		/*<< 0x7d */ _(MEMCACHED_RES_RESERVED_7d,	0, "Reserved 0x7d")			\
		/*<< 0x7e */ _(MEMCACHED_RES_RESERVED_7e,	0, "Reserved 0x7e")			\
		/*<< 0x7f */ _(MEMCACHED_RES_RESERVED_7f,	0, "Reserved 0x7f")			\
		/*<< 0x80 */ _(MEMCACHED_RES_RESERVED_80,	0, "Reserved 0x80")			\
		/*<< Reserved from 0x22 to 0x80 >>*/							\
		/* 0x81 */ _(MEMCACHED_RES_UNKNOWN_COMMAND,	1, "Unknown command")			\
		/* 0x82 */ _(MEMCACHED_RES_ENOMEM,		1, "Out of memory")			\
		/* 0x83 */ _(MEMCACHED_RES_NOT_SUPPORTED,	1, "Not supported")			\
		/* Not presented in vanilla memcached */						\
		/* 0x84 */ _(MEMCACHED_RES_SERVER_ERROR,	0, "Server error")			\
		/* Not presented in vanilla memcached */						\
		/* 0x85 */ _(MEMCACHED_RES_EBUSY,		2, "Resource busy")			\
		/* Not presented in vanilla memcached */						\
		/* 0x86 */ _(MEMCACHED_RES_EAGAIN,		2, "Temporary unavailable")

ENUM0(memcached_response, RESPONSE_CODES);

ENUM0_TXT(memcached_txt_cmd, TEXT_COMMANDS);

ENUM0_BIN(memcached_bin_cmd, BINARY_COMMANDS);

extern struct memcached_response_record memcached_response_record_table[];
extern const char *memcached_bin_cmd_name[];
extern const char *memcached_txt_cmd_name[];

static inline const char *
memcached_get_result_name(uint32_t res) {
	if (res >= memcached_response_MAX)
		return NULL;
	return memcached_response_record_table[res].name;
}

static inline int
memcached_get_result_verbosity(uint32_t res) {
	if (res >= memcached_response_MAX)
		return 0;
	return memcached_response_record_table[res].verbosity;
}

static inline const char *
memcached_get_result_description(uint32_t res) {
	if (res >= memcached_response_MAX)
		return NULL;
	return memcached_response_record_table[res].description;
}

static inline const char *
memcached_txt_cmdname(uint32_t op) {
	if (op >= memcached_txt_cmd_MAX)
		return "UNKNOWN";
	return memcached_txt_cmd_name[op];
}

static inline const char *
memcached_bin_cmdname(uint32_t op) {
	if (op >= memcached_bin_cmd_MAX)
		return "UNKNOWN";
	return memcached_bin_cmd_name[op];
}

/* BINARY STRUCTURES */

enum memcached_binary_type {
	MEMCACHED_BIN_REQUEST  = 0x80,
	MEMCACHED_BIN_RESPONSE = 0x81
};

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

/* TEXT STRUCTURES */

struct memcached_txt_request {
	enum memcached_txt_cmd op;
	const char *key;
	size_t      key_len;
	uint32_t    key_count;
	const char *data;
	size_t      data_len;
	uint64_t    flags;
	uint64_t    bytes;
	uint64_t    cas;
	uint64_t    exptime;
	uint64_t    delta;
	bool        noreply;
};

#endif /* CONSTANTS_H_INCLUDED */
