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

struct slab_arena_info *
memcached_slab_arena_info();

#endif /* ALLOC_H_INCLUDED */
