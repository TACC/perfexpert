/*
 * Copyright (c) 2011-2013 University of Texas at Austin. All rights reserved.
 * Copyright (c) 2007      Voltaire. All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology Corporation.
 *                         All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation. All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart. All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_LIST_H_
#define PERFEXPERT_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

/* Struct of list item and base type for items that are put in lists */
typedef struct perfexpert_list_item perfexpert_list_item_t;
struct perfexpert_list_item {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
};

/* Struct of a list and base type for list containers */
typedef struct perfexpert_list {
    perfexpert_list_item_t sentinel;
    volatile size_t length;
} perfexpert_list_t;

/* perfexpert_list_construct */
static inline void perfexpert_list_construct(perfexpert_list_t *list) {
    list->sentinel.next = &list->sentinel;
    list->sentinel.prev = &list->sentinel;
    list->length = 0;
}

/* perfexpert_list_destruct */
static inline void perfexpert_list_destruct(perfexpert_list_t *list) {
    perfexpert_list_construct(list);
}

/* perfexpert_list_item_construct */
static inline void perfexpert_list_item_construct(perfexpert_list_item_t *item) {
    item->next = item->prev = NULL;
}

/* perfexpert_list_item_destruct */
static inline void perfexpert_list_item_destruct(perfexpert_list_item_t *item) {
    /* nothing to do */
}

/* perfexpert_list_get_next */
#define perfexpert_list_get_next(item) ((item) ? \
    ((perfexpert_list_item_t*) ((perfexpert_list_item_t*)(item))->next) : NULL)

/* perfexpert_list_get_prev */
#define perfexpert_list_get_prev(item) ((item) ? \
    ((perfexpert_list_item_t*) ((perfexpert_list_item_t*)(item))->prev) : NULL)

/* perfexpert_list_get_first */
static inline perfexpert_list_item_t* perfexpert_list_get_first(
    perfexpert_list_t* list) {
    perfexpert_list_item_t* item = (perfexpert_list_item_t*)list->sentinel.next;
    return item;
}

/* perfexpert_list_get_size */
static inline size_t perfexpert_list_get_size(perfexpert_list_t* list) {
    return list->length;
}

/* perfexpert_list_remove_item */
static inline perfexpert_list_item_t *perfexpert_list_remove_item(
    perfexpert_list_t *list, perfexpert_list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->length--;

    return (perfexpert_list_item_t *)item->prev;
}

/* perfexpert_list_append */
static inline void perfexpert_list_append(perfexpert_list_t *list,
    perfexpert_list_item_t *item) {
    perfexpert_list_item_t *sentinel = &(list->sentinel);
    item->prev = sentinel->prev;
    sentinel->prev->next = item;
    item->next = sentinel;
    sentinel->prev = item;
    list->length++;
}

/* perfexpert_list_prepend */
static inline void perfexpert_list_prepend(perfexpert_list_t *list,
    perfexpert_list_item_t *item) {
    perfexpert_list_item_t *sentinel = &(list->sentinel);
    item->next = sentinel->next;
    item->prev = sentinel;
    sentinel->next->prev = item;
    sentinel->next = item;
    list->length++;
}

/* perfexpert_list_for */
#define perfexpert_list_for(item, list, type)  \
  for (item = (type *)  (list)->sentinel.next; \
       item != (type *) &(list)->sentinel;     \
       item = (type *) ((perfexpert_list_item_t *)(item))->next)

/* perfexpert_list_reverse_for */
#define perfexpert_list_reverse_for(item, list, type)  \
  for (item = (type *)  (list)->sentinel.prev; \
       item != (type *) &(list)->sentinel;     \
       item = (type *) ((perfexpert_list_item_t *)(item))->prev)

/* perfexpert_list_swap */
static inline void perfexpert_list_swap(perfexpert_list_item_t *a,
    perfexpert_list_item_t *b) {
    volatile perfexpert_list_item_t *temp = NULL;
    temp = a->prev;
    a->prev = b->prev;
    b->prev = temp;
    temp = a->next;
    a->next = b->next;
    b->next = temp;
}

/* perfexpert_list_move_before (move A before B) */
static inline void perfexpert_list_move_before(perfexpert_list_item_t *a,
    perfexpert_list_item_t *b) {
    a->prev->next = a->next;
    a->next->prev = a->prev;
    a->prev = b->prev;
    a->next = b;
    b->prev->next = a;
    b->prev = a;
}

/* perfexpert_list_move_after (move A after B) */
static inline void perfexpert_list_move_after(perfexpert_list_item_t *a,
    perfexpert_list_item_t *b) {
    a->prev->next = a->next;
    a->next->prev = a->prev;
    a->next = b->next;
    a->prev = b;
    b->next->prev = a;
    b->next = a;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_LIST_H */
