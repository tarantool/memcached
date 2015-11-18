#ifndef   MEMCACHED_LAYER_H_INCLUDED
#define   MEMCACHED_LAYER_H_INCLUDED

struct memcached_connection;
struct memcached_service;

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

uint64_t
convert_exptime (uint64_t exptime);

int
is_expired_tuple(struct memcached_service *p, box_tuple_t *tuple);

int
memcached_tuple_get(struct memcached_connection *con,
		    const char *key, uint32_t key_len,
		    box_tuple_t **tuple);

int
memcached_tuple_set(struct memcached_connection *con,
		    const char *kpos, uint32_t klen, uint64_t expire,
		    const char *vpos, uint32_t vlen, uint64_t cas,
		    uint32_t flags);



typedef int (* stat_func_t)(struct memcached_connection *con, const char *key,
			    const char *valfmt, ...);

int
memcached_stat_all(struct memcached_connection *con, stat_func_t append);

int
memcached_stat_reset(struct memcached_connection *con, stat_func_t append);

#endif /* MEMCACHED_LAYER_H_INCLUDED */
