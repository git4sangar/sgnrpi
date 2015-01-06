/*
 * sgn_queue.c
 *
 * SGN
 */

#include <stdlib.h>
#include <pthread.h>
#include "sgn_list.h"
#include "sgn_queue.h"

struct sgn_async_queue {
	sgn_list_t *p_head;
	sgn_list_t *p_tail;
	int queue_length;
	int waiting_thread_count;
	int ref_count;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

sgn_async_queue_t *sgn_async_queue_new() {

	/* allocate structure */
	sgn_async_queue_t *p_queue = calloc(sizeof(struct sgn_async_queue), 1);
	p_queue->p_head = p_queue->p_tail = NULL;
	p_queue->queue_length = p_queue->waiting_thread_count = 0;
	p_queue->ref_count = 1;

	/* create mutex, return null if fails */
	int ret = pthread_mutex_init(&p_queue->mutex, NULL);
	if (ret != 0) {
		free(p_queue); p_queue = NULL;
		return NULL;
	}

	/* create condition, return null if fails */
	ret = pthread_cond_init(&p_queue->cond, NULL);
	if (ret != 0) {
		pthread_mutex_destroy(&p_queue->mutex);
		free(p_queue); p_queue = NULL;
		return NULL;
	}

	return p_queue;
}

void sgn_async_queue_push(sgn_async_queue_t *p_queue, void *data) {

	if (p_queue == NULL) {
		return;
	}

	/* don't add null data */
	if (data == NULL) {
		return;
	}

	pthread_mutex_lock(&p_queue->mutex);

	/* add to the head of the queue */
	p_queue->p_head = sgn_list_prepend(p_queue->p_head, data);
	if (p_queue->p_tail == NULL) {
		/* queue was empty so tail is set to head */
		p_queue->p_tail = p_queue->p_head;
	}
	p_queue->queue_length++;

	if (p_queue->waiting_thread_count > 0) {
		pthread_cond_signal(&p_queue->cond);
	}

	pthread_mutex_unlock(&p_queue->mutex);
}

void *sgn_async_queue_pop(sgn_async_queue_t *p_queue) {

	if (p_queue == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&p_queue->mutex);

	/* wait while the queue is empty */
	while (p_queue->p_tail == NULL) {
		p_queue->waiting_thread_count++;
		pthread_cond_wait(&p_queue->cond, &p_queue->mutex);
		p_queue->waiting_thread_count--;
	}

	/* get the tail entry */
	sgn_list_t *p_node = p_queue->p_tail;
	void *data = p_node->data;

	/* remove the tail entry */
	p_queue->p_tail = p_queue->p_tail->prev;

	/* free the node structure */
	free(p_node); p_node = NULL;

	if (p_queue->p_tail != NULL) {
		/* list is not empty so fixup the tail's next pointer */
		p_queue->p_tail->next = NULL;
	} else {
		/* list is now empty so set head to null */
		p_queue->p_head = NULL;
	}

	p_queue->queue_length--;

	pthread_mutex_unlock(&p_queue->mutex);

	return data;
}

void sgn_async_queue_unref(sgn_async_queue_t *p_queue) {

	if (p_queue == NULL) {
		return;
	}

	pthread_mutex_lock(&p_queue->mutex);

	p_queue->ref_count--;

	pthread_mutex_unlock(&p_queue->mutex);

	if (p_queue->ref_count != 0) {
		return;
	} else {
		/* reference count is zero so free the queue */
		sgn_list_free(p_queue->p_head); p_queue->p_head = NULL;
		pthread_cond_destroy(&p_queue->cond);
		pthread_mutex_destroy(&p_queue->mutex);
		free(p_queue); p_queue = NULL;
	}

}


/*
void *sgn_async_queue_timed_pop(sgn_async_queue_t *p_queue, struct timespec *pTime )
{
	if ( NULL == p_queue || NULL == pTime)
		return NULL;

	pthread_mutex_lock(&p_queue->mutex);

	// wait while the queue is empty
	while (p_queue->p_tail == NULL) {
		p_queue->waiting_thread_count++;
		if( pthread_cond_timedwait(&p_queue->cond, &p_queue->mutex, pTime) )
		{
			p_queue->waiting_thread_count--;
			pthread_mutex_unlock(&p_queue->mutex);
			return NULL;
		}
		p_queue->waiting_thread_count--;
	}

	// get the tail entry
	sgn_list_t *p_node = p_queue->p_tail;
	void *data = p_node->data;

	// remove the tail entry 
	p_queue->p_tail = p_queue->p_tail->prev;

	// free the node structure 
	free(p_node);

	if (p_queue->p_tail != NULL) {
		// list is not empty so fixup the tail's next pointer 
		p_queue->p_tail->next = NULL;
	} else {
		// list is now empty so set head to null 
		p_queue->p_head = NULL;
	}

	p_queue->queue_length--;
	pthread_mutex_unlock(&p_queue->mutex);
	return data;
}

int sgn_async_queue_length(sgn_async_queue_t *p_queue) {

	if (p_queue == NULL) {
		return 0;
	}
	pthread_mutex_lock(&p_queue->mutex);
	int ret = p_queue->queue_length - p_queue->waiting_thread_count;
	pthread_mutex_unlock(&p_queue->mutex);
	return ret;
}*/
