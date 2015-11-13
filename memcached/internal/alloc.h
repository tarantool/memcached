#ifndef   ALLOC_H_INCLUDED
#define   ALLOC_H_INCLUDED

void *
box_persistent_malloc(size_t len, size_t *total);

void *
box_persistent_malloc0(size_t len, size_t *total);

void
box_persistent_free(void *ptr);

void *
box_persistent_realloc(void *ptr, size_t len, size_t *total);

void *
box_persistent_strdup(const void *str, size_t *total);

void *
box_persistent_strndup(const void *str, size_t len, size_t *total);

#endif /* ALLOC_H_INCLUDED */
