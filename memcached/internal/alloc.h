#ifndef   ALLOC_H_INCLUDED
#define   ALLOC_H_INCLUDED

void
memcached_slab_arena_create();

void
memcached_slab_arena_destroy();

void
memcached_slab_cache_create();

void
memcached_slab_cache_destroy();

struct slab_cache *
memcached_slab_cache();

#endif /* ALLOC_H_INCLUDED */
