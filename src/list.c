/* radare - LGPL - Copyright 2007-2011 pancake<nopcode.org> */

#include <string.h>
#include "list.h"

R_API void r_list_init(RList *list) {
	list->head = NULL;
	list->tail = NULL;
	list->free = NULL;
}

R_API void r_list_delete (RList *list, RListIter *iter) {
	if (iter==NULL) {
		printf ("r_list_delete: null iter?\n");
		return;
	}
list->free = free; // XXX HACK
	r_list_split_iter (list, iter);
	if (list->free && iter->data) {
		list->free (iter->data);
		iter->data = NULL;
	}
	free (iter);
}

R_API RList *r_list_new() {
	RList *list = R_NEW (RList);
	r_list_init (list);
	return list;
}

R_API void r_list_split_iter (RList *list, RListIter *iter) {
	if (list->head == iter) list->head = iter->n;
	if (list->tail == iter) list->tail = iter->p;
	if (iter->p) iter->p->n = iter->n;
	if (iter->n) iter->n->p = iter->p;
}

R_API void r_list_destroy (RList *list) {
	RListIter *it;
	if (list) {
		it = list->head;
		while (it) {
			RListIter *next = it->n;
			r_list_delete (list, it);
			it = next;
		//	free (it);
		}
		list->head = list->tail = NULL;
	}
	//free (list);
}

R_API void r_list_free (RList *list) {
	list->free = NULL;
	r_list_destroy (list);
	free (list);
}

// XXX: Too slow?
R_API RListIter *r_list_append(RList *list, void *data) {
	RListIter *new = NULL;
	if (data) {
		new = R_NEW (RListIter);
		if (list->tail)
			list->tail->n = new;
		new->data = data;
		new->p = list->tail;
		new->n = NULL;
		list->tail = new;
		if (list->head == NULL)
			list->head = new;
	}
	return new;
}

R_API RListIter *r_list_prepend(RList *list, void *data) {
	RListIter *new = R_NEW (RListIter);
	if (list->head)
		list->head->p = new;
	new->data = data;
	new->n = list->head;
	new->p = NULL;
	list->head = new;
	if (list->tail == NULL)
		list->tail = new;
	return new;
}
