#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <tarantool/lua.h>
#include <tarantool/module.h>
#include <msgpuck.h>

#include "constants.h"
#include "utils.h"

int __attribute__((unused))
memcached_strtol(const char *start, const char *end, int64_t *num)
{
	*num = 0;
	char sign = 1;
	if (*start == '-') { sign *= -1; start++; }
	while (start < end) {
		uint8_t code = *start++;
		if (code < '0' || code > '9')
			return -1;
		*num = (*num) * 10 + (code - '0');
	}
	return 0;
}

int
memcached_strtoul(const char *start, const char *end, uint64_t *num)
{
	*num = 0;
	while (start < end) {
		uint8_t code = *start++;
		if (code < '0' || code > '9')
			return -1;
		*num = (*num) * 10 + (code - '0');
	}
	return 0;
}

#define xisspace(c) isspace((unsigned char)c)

bool
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

void __attribute__((unused))
memcached_binary_header_dump(struct memcached_hdr *hdr)
{
	if (!hdr) return;
	say_debug("memcached package");
	say_debug("magic:     0x%" PRIX8,        hdr->magic);
	say_debug("cmd:       0x%" PRIX8,        hdr->cmd);
	if (hdr->key_len > 0)
		say_debug("key_len:   %" PRIu16, mp_bswap_u16(hdr->key_len));
	if (hdr->ext_len > 0)
		say_debug("ext_len:   %" PRIu8,  hdr->ext_len);
	say_debug("tot_len:   %" PRIu32,         mp_bswap_u32(hdr->tot_len));
	say_debug("opaque:    0x%" PRIX32,       mp_bswap_u32(hdr->opaque));
	say_debug("cas:       %" PRIu64,         (uint64_t )mp_bswap_u64(hdr->cas));
}

/** Find a string in an array of strings.
 *
 * @param haystack  Array of strings. Either NULL
 *                  pointer terminated (for arrays of
 *                  unknown size) or of size hmax.
 * @param needle    string to look for
 * @param hmax      the index to use if nothing is found
 *                  also limits the size of the array
 * @return  string index or hmax if the string is not found.
 */
uint32_t
strindex(const char **haystack, const char *needle, uint32_t hmax)
{
	for (unsigned index = 0; index != hmax && haystack[index]; index++)
		if (strcasecmp(haystack[index], needle) == 0)
			return index;
	return hmax;
}

/** Get box.info.ro value.
 *
 * @param result pointer to where the value will be stored.
 * @return true if the value was retrieved successfully.
*/
bool
memcached_box_is_ro(int *result)
{
	lua_State *L = luaT_state();
	int top = lua_gettop(L);
	int ro;

	lua_getfield(L, LUA_GLOBALSINDEX, "box");
	lua_getfield(L, -1, "info");
	lua_getfield(L, -1, "ro");

	if (!lua_isboolean(L, -1)) {
		return false;
	}

	ro = lua_toboolean(L, -1);
	lua_settop(L, top);
	*result = ro;

	return true;
}
