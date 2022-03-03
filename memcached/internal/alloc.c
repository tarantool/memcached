#include <tarantool/module.h>
#include <small/slab_cache.h>
#include <small/quota.h>

static struct slab_arena arena;
static struct slab_cache slabc;
static struct quota quota;

struct slab_arena_info {
	uint64_t quota_size;
	uint64_t quota_used;
	double quota_used_ratio;
};

struct slab_arena_info slab_info;

/*
 * Function produce an output with information of total quota size, used quota
 * size and quota used ratio. Note that function relies on a fact that
 * memcached uses a single slab cache for all allocations it made.
 */
struct slab_arena_info *
memcached_slab_arena_info()
{
	/*
	 * How much quota has been booked - reflects the total
	 * size of slabs in various slab caches.
	 */
	slab_info.quota_used = (uint64_t)quota_used(arena.quota);

	slab_info.quota_size = (uint64_t)quota_total(arena.quota);
	slab_info.quota_used_ratio = 100 * ((double) slab_info.quota_used /
				     ((double) slab_info.quota_size + 0.0001));

	return &slab_info;
}

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
