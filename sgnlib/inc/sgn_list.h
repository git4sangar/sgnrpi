/*
 * sgn_list.h
 *
 *  SGN
 */

/*
 * List type which works like the g_list_* equivalents.
 */
#ifndef _SGN_LIST_H_
#define _SGN_LIST_H_

typedef int (*sgn_compare_func)(void *a, void *b);
typedef struct sgn_list sgn_list_t;

struct sgn_list {
	void *data;
	sgn_list_t *next;
	sgn_list_t *prev;
};

unsigned int sgn_list_length(sgn_list_t *list);
void sgn_list_free(sgn_list_t *list);
void sgn_list_content_free(sgn_list_t *list);
sgn_list_t *sgn_list_prepend(sgn_list_t *list, void * data);
sgn_list_t *sgn_list_append(sgn_list_t *list, void * data);
sgn_list_t *sgn_list_remove(sgn_list_t *list, void * data);
sgn_list_t *sgn_list_delete_link(sgn_list_t *list, sgn_list_t *link);
sgn_list_t *sgn_list_next(sgn_list_t *list);
sgn_list_t *sgn_list_first(sgn_list_t *list);
sgn_list_t *sgn_list_last(sgn_list_t *list);
sgn_list_t *sgn_list_find(sgn_list_t *list, void *data);
sgn_list_t *sgn_list_find_custom(sgn_list_t *list, void *data, sgn_compare_func func);
sgn_list_t *sgn_list_insert_sorted(sgn_list_t *list, void * data, sgn_compare_func func);
sgn_list_t *sgn_list_sort(sgn_list_t *list, sgn_compare_func func);
void *sgn_list_nth_data( sgn_list_t *list, int n );
void sgn_list_foreach( sgn_list_t *list, void(*func)(void *, void *), void * data );
void *sgn_list_peek_head( sgn_list_t *list );
sgn_list_t *sgn_list_pop_head( sgn_list_t *list );
sgn_list_t *sgn_list_merge(sgn_list_t *list1, sgn_list_t *list2);

#endif
