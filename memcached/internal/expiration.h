#ifndef   EXPIRATION_H_INCLUDED
#define   EXPIRATION_H_INCLUDED

int
memcached_expire_start(struct memcached_service *p);

void
memcached_expire_stop(struct memcached_service *p);

#endif /* EXPIRATION_H_INCLUDED */
