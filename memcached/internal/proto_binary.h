#ifndef   PROTO_BINARY_H
#define   PROTO_BINARY_H

void
memcached_set_binary(struct memcached_connection *con);

int
memcached_binary_parse(struct memcached_connection *con);

int
memcached_binary_process(struct memcached_connection *con);

int
memcached_binary_error(struct memcached_connection *con,
		       uint16_t err, const char *errstr);

int
memcached_binary_write(struct memcached_connection *con, uint16_t err,
		       uint64_t cas, uint8_t ext_len, uint16_t key_len,
		       uint32_t val_len, const char *ext,
		       const char *key, const char *val);

int
memcached_binary_stat(struct memcached_connection *con, const char *key,
		      const char *valfmt, ...);

#endif /* PROTO_BINARY_H */
