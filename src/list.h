#ifndef _INCLUDE_R_LIST_H_
#define _INCLUDE_R_LIST_H_

#include <stdio.h>
#include "types.h"

#define R_API
// TODO: implement r_list_foreach_prev

typedef void (*SdbListFree)(void *ptr);

typedef struct r_list_iter_t {
	void *data;
	struct r_list_iter_t *n, *p;
} SdbListIter;

typedef struct r_list_t {
	struct r_list_iter_t *head;
	struct r_list_iter_t *tail;
	SdbListFree free;
} SdbList;

typedef int (*SdbListComparator)(void *a, void *b);

#ifdef R_API
#define r_list_foreach(list, it, pos) \
	for (it = list->head; it && (pos = it->data); it = it->n)
#define r_list_foreach_prev(list, it, pos) \
	for (it = list->tail; it && (pos = it->data); it = it->p)
#define r_list_iterator(x) (x)?(x)->head:NULL
#define r_list_empty(x) (x==NULL || (x->head==NULL && x->tail==NULL))
#define r_list_head(x) x->head
#define r_list_tail(x) x->tail
#define r_list_unref(x) x
#define r_list_iter_get(x) x->data; x=x->n
#define r_list_iter_next(x) (x?1:0)
#define r_list_iter_cur(x) x->p
#define r_list_iter_unref(x) x
R_API SdbList *r_list_new();
R_API SdbListIter *r_list_append(SdbList *list, void *data);
R_API SdbListIter *r_list_prepend(SdbList *list, void *data);
R_API int r_list_length(SdbList *list);
R_API void r_list_add_sorted(SdbList *list, void *data, SdbListComparator cmp);
R_API void r_list_sort(SdbList *list, SdbListComparator cmp);

R_API void r_list_delete (SdbList *list, SdbListIter *iter);
R_API boolt r_list_delete_data (SdbList *list, void *ptr);
R_API void r_list_iter_init (SdbListIter *iter, SdbList *list);
R_API void r_list_destroy (SdbList *list);
R_API void r_list_free (SdbList *list);
R_API SdbListIter *r_list_item_new (void *data);
R_API void r_list_unlink (SdbList *list, void *ptr);
R_API void r_list_split (SdbList *list, void *ptr);
R_API void r_list_split_iter (SdbList *list, SdbListIter *iter);
R_API void *r_list_get_n (SdbList *list, int n);
R_API int r_list_del_n (SdbList *list, int n);
R_API void *r_list_get_top (SdbList *list);
#define r_list_push(x,y) r_list_append(x,y)
R_API void *r_list_pop (SdbList *list);
R_API void r_list_reverse (SdbList *list);
R_API SdbList *r_list_clone (SdbList *list);

#endif

#endif
