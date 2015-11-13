#ifndef   PROTO_TEXT_PARSER_H_INCLUDED
#define   PROTO_TEXT_PARSER_H_INCLUDED

struct memcached_text_request;

int
memcached_text_parser(struct memcached_connection *con,
		      const char **p_ptr, const char *pe);

#endif /* PROTO_TEXT_PARSER_H_INCLUDED */
