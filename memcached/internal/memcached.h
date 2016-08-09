#ifndef   MEMCACHED_H_INCLUDED
#define   MEMCACHED_H_INCLUDED

/*
 ** Old text memcached API
 * int
 * memcached_parse_text(struct mc_request *req,
 * 		     const char **p,
 * 		     const char *pe);
 */


#include <small/obuf.h>
#include "constants.h"

struct memcached_connection;

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

enum memcached_proto_type {
	MEMCACHED_PROTO_NEGOTIATION = 0x00,
	MEMCACHED_PROTO_BINARY      = 0x01,
	MEMCACHED_PROTO_TEXT        = 0x02,
	MEMCACHED_PROTO_MAX
};

struct memcached_stat {
	/* connection informations */
	unsigned int  curr_items;
	unsigned int  total_items;
	unsigned int  curr_conns;
	unsigned int  total_conns;
	uint64_t      bytes_read;
	uint64_t      bytes_written;
	/* get statistics */
	uint64_t      cmd_get;
	uint64_t      get_hits;
	uint64_t      get_misses;
	/* delete stats */
	uint64_t      cmd_delete;
	uint64_t      delete_hits;
	uint64_t      delete_misses;
	/* set statistics */
	uint64_t      cmd_set;
	uint64_t      cas_hits;
	uint64_t      cas_badval;
	uint64_t      cas_misses;
	/* incr/decr stats */
	uint64_t      cmd_incr;
	uint64_t      incr_hits;
	uint64_t      incr_misses;
	uint64_t      cmd_decr;
	uint64_t      decr_hits;
	uint64_t      decr_misses;
	/* touch/flush stats */
	uint64_t      cmd_touch;
	uint64_t      touch_hits;
	uint64_t      touch_misses;
	uint64_t      cmd_flush;
	/* expiration stats */
	uint64_t      evictions;
	uint64_t      reclaimed;
	/* authentication stats */
	uint64_t      auth_cmds;
	uint64_t      auth_errors;
};

struct memcached_service {
	/* expiration configuration */
	struct fiber *expire_fiber;
	bool          expire_enabled;
	int           expire_count;
	uint32_t      expire_time;
	/* flush */
	bool          flush_enabled;
	int           batch_count;
	/* configurable */
	int           readahead;
	const char   *uri;
	const char   *name;
	uint32_t      space_id;
	bool          sasl;
	/* properties */
	uint64_t      cas;
	uint64_t      flush;
	int           verbosity;
	enum memcached_proto_type proto;
	struct memcached_stat     stat;
};

typedef int (* memcached_loop_func_t)(struct memcached_connection *con);

typedef int (* memcached_error_func_t)(struct memcached_connection *con,
				       uint16_t err, const char *errstr);

/**
 * Single connection object, handles information about
 * 1) pointer to memcached stats
 * 2) pointer to memcached config
 * 3) connection data, that's created by coio
 * 4) internal tarantool session for access limitations
 * 4) last decoded memcached message (we can do it since memcached
 * 				      binary proto is synchronious)
 */
struct memcached_connection {
	/* memcached_specific data */
	int                       fd;
	struct memcached_service *cfg;
	/* connection data */
	struct ibuf              *in;
	struct obuf              *out;
	struct obuf_svp           write_end;
	bool                      noreply;
	bool                      noprocess;
	bool                      close_connection;
	/* session data */
//	union {
//		struct sockaddr addr;
//		struct sockaddr_storage addrstorage;
//	};
//	socklen_t addr_len;
//	struct session           *session;
	struct sasl_ctx          *sasl_ctx;
	int                       authentication_step;
	union {
		/* request data (binary) */
		struct {
			struct memcached_hdr  *hdr;
			struct memcached_body  body;
		};
		/* request data (text) */
		struct memcached_txt_request   request;
	}
	/* enum memcached_response */;
	int                       errcode;
	/* length of package */
	size_t                    len;
	struct {
		memcached_loop_func_t     parse_request;
		memcached_loop_func_t     process_request;
		memcached_error_func_t    process_error;
	} cb; /* protocol specific callbacks */
};

struct memcached_stat *
memcached_get_stat(struct memcached_service *);

struct memcached_service *
memcached_create(const char *, uint32_t);

int  memcached_start(struct memcached_service *);
void memcached_stop(struct memcached_service *);
void memcached_free(struct memcached_service *);

void memcached_handler(struct memcached_service *p, int fd);


/* options */
enum memcached_options {
	MEMCACHED_OPT_READAHEAD      = 0x00,
	MEMCACHED_OPT_EXPIRE_ENABLED = 0x01,
	MEMCACHED_OPT_EXPIRE_COUNT   = 0x02,
	MEMCACHED_OPT_EXPIRE_TIME    = 0x03,
	MEMCACHED_OPT_FLUSH_ENABLED  = 0x04,
	MEMCACHED_OPT_VERBOSITY      = 0x05,
	MEMCACHED_OPT_PROTOCOL       = 0x06,
	MEMCACHED_OPT_SASL           = 0x07,
	MEMCACHED_OPT_MAX
};

void memcached_set_opt(struct memcached_service *, int, ...);

#define MEMCACHED_MAX_SIZE (1 << 20)

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* MEMCACHED_H_INCLUDED */
