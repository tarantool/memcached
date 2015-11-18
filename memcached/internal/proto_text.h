#ifndef   PROTO_TEXT_H_INCLUDED
#define   PROTO_TEXT_H_INCLUDED

typedef int (* mc_process_func_t)(struct memcached_connection *con);

extern const mc_process_func_t memcached_txt_handler[];

void
memcached_set_text(struct memcached_connection *con);

int
memcached_txt_process_unknown(struct memcached_connection *con);

#endif /* PROTO_TEXT_H_INCLUDED */
