#ifndef   MEMCACHED_UTIL_H
#define   MEMCACHED_UTIL_H

#include <stdbool.h>
#include <stdint.h>

int
memcached_strtol(const char *start, const char *end, int64_t *num);

int
memcached_strtoul(const char *start, const char *end, uint64_t *num);

bool
safe_strtoull(const char *begin, const char *end, uint64_t *out);

void
memcached_binary_header_dump(struct memcached_hdr *hdr);

#endif /* MEMCACHED_UTIL_H */
