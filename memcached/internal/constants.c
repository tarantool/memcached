#include "constants.h"

#define MEMCACHED_RESPONSE_RECORD_MEMBER(s, v, d) {	\
	.name        = #s,				\
	.description = d,				\
	.verbosity   = v,				\
},

#define MEMCACHED_COMMAND_MEMBER(s) #s,

struct memcached_response_record memcached_response_record_table[] = {
	RESPONSE_CODES(MEMCACHED_RESPONSE_RECORD_MEMBER)
};

const char *memcached_txt_cmd_name[] = {
	TEXT_COMMANDS(MEMCACHED_COMMAND_MEMBER)
	NULL
};

const char *memcached_bin_cmd_name[] = {
	BINARY_COMMANDS(MEMCACHED_COMMAND_MEMBER)
	NULL
};
