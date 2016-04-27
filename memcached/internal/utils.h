#ifndef   UTILS_H_INCLUDED
#define   UTILS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

struct memcached_hdr;

int
memcached_strtol(const char *start, const char *end, int64_t *num);

int
memcached_strtoul(const char *start, const char *end, uint64_t *num);

bool
safe_strtoull(const char *begin, const char *end, uint64_t *out);

void
memcached_binary_header_dump(struct memcached_hdr *hdr);

/* Macros to define enum and corresponding strings. */
#define ENUM0_MEMBER(s, ...) s,
#define ENUM_MEMBER(s, v, ...) s = v,
#define ENUM0(enum_name, enum_members) enum enum_name { enum_members(ENUM0_MEMBER) enum_name##_MAX }
#define ENUM(enum_name, enum_members) enum enum_name { enum_members(ENUM_MEMBER) enum_name##_MAX }
#if defined(__cplusplus)
#define ENUM_STRS_MEMBER(s, v, ...) names[s] = #s;
/* A special hack to emulate C99 designated initializers */
#define STRS(enum_name, enum_members)					\
	const char *enum_name##_strs[enum_name##_MAX];			\
	namespace {							\
		const struct enum_name##_strs_init {			\
			enum_name##_strs_init(const char **names) {	\
				memset(names, 0, sizeof(*names) *	\
				       enum_name##_MAX);		\
				enum_members(ENUM_STRS_MEMBER)		\
			}						\
		}  enum_name##_strs_init(enum_name##_strs);		\
	}
#else /* !defined(__cplusplus) */
#define ENUM_STRS_MEMBER(s, v, ...) [s] = #s,
#define STRS(enum_name, enum_members) \
	const char *enum_name##_strs[(unsigned) enum_name##_MAX + 1] = {enum_members(ENUM_STRS_MEMBER) 0}
#endif
#define STR2ENUM(enum_name, str) ((enum enum_name) strindex(enum_name##_strs, str, enum_name##_MAX))

uint32_t
strindex(const char **haystack, const char *needle, uint32_t hmax);

#endif /* UTILS_H_INCLUDED */
