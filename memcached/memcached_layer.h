#ifndef   TARANTOOL_BOX_MEMCACHED_LAYER_H_INCLUDED
#define   TARANTOOL_BOX_MEMCACHED_LAYER_H_INCLUDED

enum memcached_set {
	MEMCACHED_SET_CAS = 0,
	MEMCACHED_SET_ADD,
	MEMCACHED_SET_SET,
	MEMCACHED_SET_REPLACE
};

typedef int (* mc_process_func_t)(struct memcached_connection *con);

int memcached_bin_process_unknown(struct memcached_connection *con);

int memcached_bin_error(struct memcached_connection *con,
		    uint16_t err, const char *errstr);
int memcached_bin_errori(struct memcached_connection *con);

int
is_expired_tuple(struct memcached_service *p, box_tuple_t *tuple);

extern const mc_process_func_t mc_handler[];

#define memcached_error_ENOMEM(_bytes, _place, _for) \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_ENOMEM, \
			"Failed to allocate %u bytes in '%s' for %s", \
			(_bytes), (_place), (_for))

#define memcached_error_KEY_ENOENT() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_KEY_ENOENT, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_KEY_ENOENT])

#define memcached_error_KEY_EEXISTS() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_KEY_EEXISTS, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_KEY_EEXISTS])

#define memcached_error_E2BIG() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_E2BIG, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_E2BIG])

#define memcached_error_EINVAL() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_EINVAL, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_EINVAL])

#define memcached_error_NOT_STORED() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_NOT_STORED, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_NOT_STORED])

#define memcached_error_DELTA_BADVAL() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_DELTA_BADVAL, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_DELTA_BADVAL])

#define memcached_error_AUTH_ERROR() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_AUTH_ERROR, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_AUTH_ERROR])

#define memcached_error_AUTH_CONTINUE() \
	box_error_raise(box_error_code_MAX + MEMCACHED_BIN_RES_AUTH_CONTINUE, \
		memcached_binary_res_title[MEMCACHED_BIN_RES_AUTH_CONTINUE])

#define memcached_error_UNKNOWN_COMMAND(code) do {				\
		box_error_raise(box_error_code_MAX +				\
				MEMCACHED_BIN_RES_UNKNOWN_COMMAND,		\
				"Unknown command with opcode 0x%.2x", (code));	\
		say_error("Unknown command with opcode 0x%.2x", (code));	\
	} while (0)

#define memcached_error_NOT_SUPPORTED(op) do {					\
		box_error_raise(box_error_code_MAX +				\
				MEMCACHED_BIN_RES_NOT_SUPPORTED,		\
				"Unsupported command '%s'", (op));		\
		say_error("Unsupported command '%s'", (op));			\
	} while (0)

#define memcached_error_SERVER_ERROR(fmtstr, ...) do {				\
		box_error_raise(box_error_code_MAX +				\
				MEMCACHED_BIN_RES_SERVER_ERROR,			\
				(fmtstr), ##__VA_ARGS__);			\
		say_error((fmtstr), ##__VA_ARGS__);				\
	} while (0)

#endif /* TARANTOOL_BOX_MEMCACHED_LAYER_H_INCLUDED */
