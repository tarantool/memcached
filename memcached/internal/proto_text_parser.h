#ifndef   TARANTOOL_BOX_MEMCACHED_TEXT_H_INCLUDED
#define   TARANTOOL_BOX_MEMCACHED_TEXT_H_INCLUDED

struct memcached_text_request;

int
memcached_text_parse(struct memcached_text_request *req, const char **p,
		     const char *pe);

#endif /* TARANTOOL_BOX_MEMCACHED_TEXT_H_INCLUDED */
