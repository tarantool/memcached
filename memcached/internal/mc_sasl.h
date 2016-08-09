#ifndef   MC_SASL_H_INCLUDED
#define   MC_SASL_H_INCLUDED

struct sasl_conn;
typedef struct sasl_conn sasl_conn_t;

/*
 * struct sasl_conn;
 * typedef struct sasl_conn sasl_conn_t;
 * typedef int (sasl_callback_ft )(void);
 */

struct sasl_ctx {
	sasl_conn_t *sasl_conn;
};

int memcached_sasl_init(void);
int memcached_sasl_connection_init(struct memcached_connection *con);
int memcached_sasl_list_mechs(struct memcached_connection *con,
			      const char **mechs, size_t *mechs_len);
int memcached_sasl_auth(struct memcached_connection *con, const char *mech,
			const char *challenge, size_t challenge_len,
			const char **out, size_t *out_len);
int memcached_sasl_step(struct memcached_connection *con,
			const char *challenge, size_t challenge_len,
			const char **out, size_t *out_len);

#endif /* MC_SASL_H_INCLUDED */
