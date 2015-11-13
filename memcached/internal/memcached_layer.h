#ifndef   MEMCACHED_LAYER_H_INCLUDED
#define   MEMCACHED_LAYER_H_INCLUDED

struct memcached_connection;

enum memcached_set {
	MEMCACHED_SET_CAS = 0,
	MEMCACHED_SET_ADD,
	MEMCACHED_SET_SET,
	MEMCACHED_SET_REPLACE
};

typedef int (* mc_process_func_t)(struct memcached_connection *con);

int
memcached_process_unsupported(struct memcached_connection *con);

int
memcached_process_unknown(struct memcached_connection *con);

int
is_expired_tuple(struct memcached_service *p, box_tuple_t *tuple);

int
memcached_bin_process_set(struct memcached_connection *con);
int
memcached_bin_process_get(struct memcached_connection *con);
int
memcached_bin_process_delete(struct memcached_connection *con);
int
memcached_bin_process_version(struct memcached_connection *con);
int
memcached_bin_process_noop(struct memcached_connection *con);
int
memcached_bin_process_flush(struct memcached_connection *con);
int
memcached_bin_process_verbosity(struct memcached_connection *con);
int
memcached_bin_process_gat(struct memcached_connection *con);
int
memcached_bin_process_delta(struct memcached_connection *con);
int
memcached_bin_process_pend(struct memcached_connection *con);
int
memcached_bin_process_quit(struct memcached_connection *con);
int
memcached_bin_process_stat(struct memcached_connection *con);

#endif /* MEMCACHED_LAYER_H_INCLUDED */
