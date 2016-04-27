#ifndef   ERROR_H_INCLUDED
#define   ERROR_H_INCLUDED

#define memcached_error_ENOMEM(_bytes, _for)				\
	box_error_raise(box_error_code_MAX + MEMCACHED_RES_ENOMEM,	\
			"Failed to allocate %u bytes in '%s' for %s",	\
			(_bytes), __func__, (_for))

/*
 * Use memcached_error(code) instead of:
 *
 * memcached_error_KEY_ENOENT()
 * memcached_error_KEY_EEXISTS()
 * memcached_error_E2BIG()
 * memcached_error_EINVAL()
 * memcached_error_NOT_STORED()
 * memcached_error_DELTA_BADVAL()
 * memcached_error_AUTH_ERROR()
 * memcached_error_AUTH_CONTINUE()
 */

#define memcached_error(_code)								\
	box_error_raise(box_error_code_MAX + (_code),					\
			memcached_get_result_description((_code)))

#define memcached_set_errcode(_con, _error)						\
	(_con)->errcode = box_error_code_MAX + (_error)

#define memcached_error_EINVALS(_error)							\
	box_error_raise(box_error_code_MAX + MEMCACHED_RES_EINVAL, (_error))

#define memcached_error_UNKNOWN_COMMAND(_code) do {					\
		box_error_raise(box_error_code_MAX + MEMCACHED_RES_UNKNOWN_COMMAND,	\
				"Unknown command with opcode 0x%.2x", (_code));		\
		say_error("Unknown command with opcode 0x%.2x", (_code));		\
	} while (0)

#define memcached_error_txt_UNKNOWN_COMMAND(_str, _len) do {				\
		box_error_raise(box_error_code_MAX + MEMCACHED_RES_UNKNOWN_COMMAND,	\
				"Unknown command with name '%.*s'", (_len), (_str));	\
		say_error("Unknown command with name '%.*s'", (_len), (_str));		\
	} while (0)

#define memcached_error_NOT_SUPPORTED(op) do {						\
		box_error_raise(box_error_code_MAX + MEMCACHED_RES_NOT_SUPPORTED,	\
				"Unsupported command '%s'", (op));			\
		say_error("Unsupported command '%s'", (op));				\
	} while (0)

#define memcached_error_SERVER_ERROR(fmtstr, ...) do {					\
		box_error_raise(box_error_code_MAX + MEMCACHED_RES_SERVER_ERROR,	\
				(fmtstr), ##__VA_ARGS__);				\
		say_error((fmtstr), ##__VA_ARGS__);					\
	} while (0)

#endif /* ERROR_H_INCLUDED */
