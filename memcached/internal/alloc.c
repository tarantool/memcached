#include <tarantool/module.h>
#include <small/slab_cache.h>
#include <small/quota.h>

static struct slab_arena arena;
static struct slab_cache slabc;
static struct quota quota;

void
memcached_slab_arena_create()
{
	static __thread bool inited = false;
	if (inited) return;
	size_t quota_size = QUOTA_MAX;
	quota_init(&quota, quota_size);
	/* Parameters are the same as in src/lib/core/memory.c:memory_init() */
	const size_t SLAB_SIZE = 4 * 1024 * 1024;
	slab_arena_create(&arena, &quota, 0, SLAB_SIZE, SLAB_ARENA_PRIVATE);
	say_info("creating arena with %zu bytes...", quota_size);
	inited = 1;
}

void
memcached_slab_arena_destroy()
{
	static __thread bool freed = false;
	if (freed) return;
	say_info("destroying arena...");
	slab_arena_destroy(&arena);
	freed = 1;
}

void
memcached_slab_cache_create()
{
	static __thread bool inited = false;
	if (inited) return;
	slab_cache_set_thread(&slabc);
	slab_cache_create(&slabc, &arena);
	say_info("allocate slab cache with slab size %u", arena.slab_size);
	inited = 1;
}

void
memcached_slab_cache_destroy()
{
	static __thread bool freed = false;
	if (freed) return;
	say_info("destroying slab cache");
	slab_cache_destroy(&slabc);
	freed = 1;
}

struct slab_cache *
memcached_slab_cache()
{
	return &slabc;
}
