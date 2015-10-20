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

int memcached_error(struct memcached_connection *con,
		    uint16_t err, const char *errstr);
int memcached_errori(struct memcached_connection *con);

int
is_expired_tuple(struct memcached_service *p, box_tuple_t *tuple);

extern const mc_process_func_t mc_handler[];

#endif /* TARANTOOL_BOX_MEMCACHED_LAYER_H_INCLUDED */
