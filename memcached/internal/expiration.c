#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>

#include <tarantool/module.h>
#include <msgpuck.h>

#include "memcached.h"
#include "memcached_layer.h"

#include "error.h"

int
memcached_expire_process(struct memcached_service *p, box_iterator_t **iterp)
{
	box_iterator_t *iter = *iterp;
	box_tuple_t *tpl = NULL;
	box_txn_begin();
	int i = 0;
	for (i = 0; i < p->expire_count; ++i) {
		if (box_iterator_next(iter, &tpl) == -1) {
			box_txn_rollback();
			return -1;
		} else if (tpl == NULL) {
			box_iterator_free(iter);
			box_txn_commit();
			*iterp = NULL;
			return 0;
		} else if (is_expired_tuple(p, tpl)) {
			uint32_t klen = 0;
			const char *kpos = box_tuple_field(tpl, 0);
			            kpos = mp_decode_str(&kpos, &klen);
			size_t sz   = mp_sizeof_array(1) + mp_sizeof_str(klen);
			char *begin = (char *)box_txn_alloc(sz);
			if (begin == NULL) {
				box_txn_rollback();
				memcached_error_ENOMEM(sz, "key");
				return -1;
			}
			char *end = mp_encode_array(begin, 1);
			      end = mp_encode_str(end, kpos, klen);
			if (box_delete(p->space_id, 0, begin, end, NULL)) {
				box_txn_rollback();
				return -1;
			}
			p->stat.evictions++;
		}
	}
	box_txn_commit();
	return 0;
}

int
memcached_expire_loop(va_list ap)
{
	struct memcached_service *p = va_arg(ap, struct memcached_service *);
	char key[2], *key_end = mp_encode_array(key, 0);
	box_iterator_t *iter = NULL;
	int rv = 0;
	say_info("Memcached expire fiber started");
restart:
	if (iter == NULL) {
		iter = box_index_iterator(p->space_id, 0, ITER_ALL, key, key_end);
	}
	if (rv == -1 || iter == NULL) {
		const box_error_t *err = box_error_last();
		say_error("Unexpected error %u: %s",
				box_error_code(err),
				box_error_message(err));
		goto finish;
	}
	rv = memcached_expire_process(p, &iter);

	/* This part is where we rest after all deletes */
	double delay = ((double )p->expire_count * p->expire_time) /
			(box_index_len(p->space_id, 0) + 1);
	if (delay > 1) delay = 1;
	fiber_set_cancellable(true);
	fiber_sleep(delay);
	if (fiber_is_cancelled())
		goto finish;
	fiber_set_cancellable(false);

	goto restart;
finish:
	if (iter) box_iterator_free(iter);
	return 0;
}

int
memcached_expire_start(struct memcached_service *p)
{
	if (p->expire_fiber != NULL)
		return -1;
	struct fiber *expire_fiber = NULL;
	char name[128];
	snprintf(name, 128, "__mc_%s_expire", p->name);
	expire_fiber = fiber_new(name, memcached_expire_loop);
	const box_error_t *err = box_error_last();
	if (err) {
		say_error("Can't start the expire fiber");
		say_error("%s", box_error_message(err));
		return -1;
	}
	p->expire_fiber = expire_fiber;
	fiber_set_joinable(expire_fiber, true);
	fiber_start(expire_fiber, p);
	return 0;
}

void
memcached_expire_stop(struct memcached_service *p)
{
	if (p->expire_fiber == NULL) return;
	fiber_cancel(p->expire_fiber);
	fiber_join(p->expire_fiber);
	p->expire_fiber = NULL;
}

