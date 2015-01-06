/*
 * sgn_list.c
 * sgn
 */

#include <stdlib.h>
#include "sgn_list.h"

unsigned int sgn_list_length(sgn_list_t *list) {
	int ret = 0;

	while (list != NULL) {
		ret++;
		list = list->next;
	}
	return ret;
}

void sgn_list_free(sgn_list_t *list) {

	sgn_list_t *prev;
	while (list != NULL) {
		prev = list;
		list = list->next;
		free(prev); prev = NULL;
	}
}

void sgn_list_content_free(sgn_list_t *list) {
	list = sgn_list_first(list);
	while(list) {
		free(list->data); list->data = NULL;
		list = list->next;
	}
}

sgn_list_t *sgn_list_prepend(sgn_list_t *list, void *data) {
	sgn_list_t *pNew = calloc(sizeof(sgn_list_t),1);
	pNew->data = data;
	pNew->next = list;

	if (list != NULL) {
		/* handle like insert in case the pointer to list is really not the first */
		pNew->prev = list->prev;
		if (list->prev != NULL) {
			list->prev->next = pNew;
		}
		list->prev = pNew;
	} else {
		pNew->prev = NULL;
	}

	return pNew;
}

sgn_list_t *sgn_list_append(sgn_list_t *list, void *data) {

	sgn_list_t *pNew = calloc(sizeof(sgn_list_t),1);
	pNew->data = data;
	pNew->next = NULL;

	if (list != NULL) {
		/* find last element and append to it */
		sgn_list_t *last = sgn_list_last(list);
		last->next = pNew;
		pNew->prev = last;
		return list;
	} else {
		/* no list, pNew element is the list so return it */
		pNew->prev = NULL;
		return pNew;
	}
}

sgn_list_t *sgn_list_remove(sgn_list_t *list, void *data) {

	sgn_list_t *tmp = list;
	while (tmp != NULL) {
		if (tmp->data == data) {
			/* found the element, remove from the list */
			if (tmp->prev != NULL) {
				tmp->prev->next = tmp->next;
			}
			if (tmp->next != NULL) {
				tmp->next->prev = tmp->prev;
			}

			/* if this is the first element, return the next */
			if (list == tmp) {
				list = tmp->next;
			}

			/* free the element, set tmp to NULL to exit loop */
			free(tmp); tmp = NULL;

		} else {
			/* go to the next node */
			tmp = tmp->next;
		}
	}

	return list;
}

sgn_list_t *sgn_list_delete_link(sgn_list_t *list, sgn_list_t *link) {

	if (link != NULL) {
		/* remove the element from the list */
		if (link->prev != NULL) {
			link->prev->next = link->next;
		}
		if (link->next != NULL) {
			link->next->prev = link->prev;
		}

		/* if this link is the first, point to the next */
		if (list == link) {
			list = link->next;
		}
		free(link); link = NULL;
	}
	return list;
}

sgn_list_t *sgn_list_next(sgn_list_t *list) {

	if (list != NULL) {
		return list->next;
	} else {
		return list;
	}
}

sgn_list_t *sgn_list_first(sgn_list_t *list) {
	if (list != NULL) {
		while (list->prev != NULL) {
			list = list->prev;
		}
	}
	return list;
}

sgn_list_t *sgn_list_last(sgn_list_t *list) {
	if (list != NULL) {
		while (list->next != NULL) {
			list = list->next;
		}
	}
	return list;
}

sgn_list_t *sgn_list_find(sgn_list_t *list, void *data) {
	while (list != NULL) {
		if (list->data == data) {
			return list;
		} else {
			list = list->next;
		}
	}
	return NULL;
}

sgn_list_t *sgn_list_find_custom(sgn_list_t *list, void *data, sgn_compare_func func) {
	/* return NULL if no function */
	if (func == NULL) {
		return NULL;
	}
	while (list != NULL) {
		if (!func(list->data, data)) {
			return list;
		} else {
			list = list->next;
		}
	}
	return NULL;
}

sgn_list_t *sgn_list_insert_sorted(sgn_list_t *list, void *data, sgn_compare_func func) {
	/* return list if no function */
	if (func == NULL) {
		return list;
	}

	/* allocate the element */
	sgn_list_t *pNew = calloc(sizeof(sgn_list_t),1);
	pNew->data = data;
	pNew->next = NULL;
	pNew->prev = NULL;

	/* if the list is null the pNew element is the list */
	if (list == NULL) {
		return pNew;
	}

	/* get pointer to traverse list */
	sgn_list_t *tmp = list;
	/* check relative to first element */
	int cmp = func(data, tmp->data);

	/* continue to next element while compare is > 0 */
	while (cmp > 0 && tmp->next != NULL) {
		/* point to next element and do compare again */
		tmp = tmp->next;
		cmp = func(data, tmp->data);
	}

	/* if we have stopped because we are at the end of the
	 *  list and the compare is > 0, append the pNew element */
	if (tmp->next == NULL && cmp > 0) {
		tmp->next = pNew;
		pNew->prev = tmp;
		return list;
	}

	/* link to any previous element */
	if (tmp->prev != NULL) {
		tmp->prev->next = pNew;
		pNew->prev = tmp->prev;
	}
	/* and link to the next element */
	pNew->next = tmp;
	tmp->prev = pNew;

	/* if we were pointing to the head of the list return the
	 * pNew element since it is now the head. */
	if (tmp == list) {
		return pNew;
	} else {
		return list;
	}
}

sgn_list_t *sgn_list_sort(sgn_list_t *list, sgn_compare_func func) {
	/* return list if no compare function */
	if (func == NULL) {
		return list;
	}

	/* use insert sorted function to do an insertion sort into a pNew list */
	sgn_list_t *new_list = NULL;
	sgn_list_t *iter = list;
	while (iter != NULL) {
		new_list = sgn_list_insert_sorted(new_list, iter->data, func);
		iter = sgn_list_next(iter);
	}

	/* free the original list */
	sgn_list_free(list); list = NULL;

	return new_list;
}

void sgn_list_foreach( sgn_list_t *list, void(*func)(void *, void *), void *data ) {
	list = sgn_list_first( list );
	while( NULL != list )
	{
		func( list->data, data );
		list = sgn_list_next( list );
	}
}

void *sgn_list_nth_data( sgn_list_t *list, int n )
{
	sgn_list_t 	*pNthEle;
	int			i32Count;

	if( NULL == list || 0 >= n || sgn_list_length(list) < n)
		return NULL;

	pNthEle	= list;
	pNthEle = sgn_list_first( pNthEle );

	i32Count = 1;
	while( i32Count++ < n && pNthEle )
		pNthEle	= sgn_list_next( pNthEle );

	return pNthEle->data;
}

void *sgn_list_peek_head( sgn_list_t *list ) {
	void *pData = NULL;

	list	= sgn_list_first( list );
	if( list )
		pData	= list->data;

	return pData;
}

sgn_list_t *sgn_list_pop_head( sgn_list_t *list ) {
	list	= sgn_list_first( list );
	list	= sgn_list_remove( list, list->data );

	return list;
}

sgn_list_t *sgn_list_merge(sgn_list_t *list1, sgn_list_t *list2) {
	if(!list2) {
		return list1;
	}

	if(!list1) {
		list1 = list2;
		return sgn_list_last(list1);
	}

	list1	= sgn_list_last(list1);
	list2	= sgn_list_first(list2);

	list1->next	= list2;
	list2->prev	= list1;

	return sgn_list_last(list1);
}

