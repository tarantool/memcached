#ifndef TARANTOOL_MEMCACHED_H_INCLUDED
#define TARANTOOL_MEMCACHED_H_INCLUDED

/*
 ** Old text memcached API
 * int
 * memcached_parse_text(struct mc_request *req,
 * 		     const char **p,
 * 		     const char *pe);
 */

#include "memcached_constants.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

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
	struct fiber         *expire_fiber;
	bool                  expire_enabled;
	uint32_t              expire_count;
	uint32_t              expire_time;
	/* flush */
	bool                  flush_enabled;
	uint32_t              batch_count;
	/* configurable */
	int                   readahead;
	const char           *uri;
	const char           *name;
	uint32_t              space_id;
	/* properties */
	uint64_t              cas;
	uint64_t              flush;
	struct memcached_stat stat;
	int                   verbosity;
	/**
	 *  0 if we need to continue transaction
	 *  1 if transaction must be commited
	 * -1 if transaction must be aborted
	 **/
	int                   intxn;
};

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
	struct iobuf             *iobuf;
	struct obuf_svp           write_end;
	bool                      noreply;
	bool                      close_connection;
	bool                      noprocess;
	/* session data */
//	union {
//		struct sockaddr addr;
//		struct sockaddr_storage addrstorage;
//	};
//	socklen_t addr_len;
//	struct session           *session;
	/* request data */
	struct memcached_hdr     *hdr;
	struct memcached_body     body;
	size_t                    len;
};

enum memcached_options {
	MEMCACHED_OPT_READAHEAD = 0,
	MEMCACHED_OPT_EXPIRE_ENABLED,
	MEMCACHED_OPT_EXPIRE_COUNT,
	MEMCACHED_OPT_EXPIRE_TIME,
	MEMCACHED_OPT_FLUSH_ENABLED,
	MEMCACHED_OPT_VERBOSITY,
};

void memcached_set_opt(struct memcached_service *, int, ...);

struct memcached_stat *
memcached_get_stat(struct memcached_service *);

struct memcached_service *
memcached_create(const char *, uint32_t);

void memcached_start(struct memcached_service *);
void memcached_stop(struct memcached_service *);
void memcached_free(struct memcached_service *);

void memcached_expire_start(struct memcached_service *p);
void memcached_expire_stop(struct memcached_service *p);

void memcached_handler(struct memcached_service *p, int fd);

#define MEMCACHED_MAX_SIZE (1 << 20)

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif
