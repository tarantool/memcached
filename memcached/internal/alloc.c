#include <module.h>
#include <small/slab_cache.h>

#include "memcached_persistent.h"

void *
box_persistent_malloc(size_t len, size_t *total)
{
	struct slab *slab = slab_get(cord_slab_cache(), len);
	char *ptr = NULL; size_t sz = 0;
	if (slab != NULL) {
		ptr = slab_data(slab);
		sz  = slab_capacity(slab);
	}
	if (total) *total = sz;
	return ptr;
}

void *
box_persistent_malloc0(size_t len, size_t *total)
{
	size_t sz = 0;
	char *ptr = box_persistent_malloc(len, &sz);
	if (ptr) memset(ptr, 0, sz);
	if (total) *total = sz;
	return ptr;
}

void
box_persistent_free(void *ptr) {
	if (ptr) slab_put(cord_slab_cache(), slab_from_data(ptr));
}

void *
box_persistent_realloc(void *ptr, size_t len, size_t *total)
{
	if (len == 0) {
		if (ptr) box_persistent_free(ptr);
		if (total) *total = 0;
		return NULL;
	} else if (ptr == NULL) {
		return box_persistent_malloc(len, total);
	}
	struct slab *slab = slab_from_data(ptr);
	size_t sz = slab_capacity(slab);
	if (len <= sz) {
		if (total) *total = sz;
		return ptr;
	}
	char *ptr_new = box_persistent_malloc(len, total);
	if (!ptr_new)
		return NULL;
	memcpy(ptr_new, ptr, sz);
	box_persistent_free(ptr);
	return ptr_new;
}

void *
box_persistent_strdup(const void *str, size_t *total)
{
	size_t strl = strlen(str);
	char *ptr = box_persistent_malloc(strl, total);
	if (ptr) {
		memcpy(ptr, str, strl);
		ptr[strl] = 0;
	}
	return ptr;
}

void *
box_persistent_strndup(const void *str, size_t len, size_t *total)
{
	char *ptr = box_persistent_malloc(len, total);
	if (ptr) strncpy(ptr, str, len);
	return ptr;
}
