/*
 * SGN
 * sgn_queue.h
 *
 */

#ifndef _SGN_QUEUE_H_
#define _SGN_QUEUE_H_

typedef struct sgn_async_queue sgn_async_queue_t;

sgn_async_queue_t *sgn_async_queue_new();
void sgn_async_queue_push(sgn_async_queue_t *p_queue, void *data);
void *sgn_async_queue_pop(sgn_async_queue_t *p_queue);
void sgn_async_queue_unref(sgn_async_queue_t *p_queue);

/* y_generic_ptr sgn_async_queue_timed_pop(sgn_async_queue_t *p_queue, struct timespec *pTime );
int sgn_async_queue_length(sgn_async_queue_t *p_queue); */

#endif /* _SGN_QUEUE_H_ */
