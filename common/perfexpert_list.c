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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* perfexpert_list_construct */
void perfexpert_list_construct(perfexpert_list_t *list) {
    list->sentinel.next = &list->sentinel;
    list->sentinel.prev = &list->sentinel;
    list->length = 0;
}

/* perfexpert_list_destruct */
void perfexpert_list_destruct(perfexpert_list_t *list) {
    perfexpert_list_construct(list);
}

/* perfexpert_list_item_construct */
void perfexpert_list_item_construct(perfexpert_list_item_t *item) {
    item->next = item->prev = NULL;
}

/* perfexpert_list_item_destruct */
void perfexpert_list_item_destruct(perfexpert_list_item_t *item) {
    /* nothing to do */
}

/* perfexpert_list_get_first */
perfexpert_list_item_t* perfexpert_list_get_first(perfexpert_list_t* list) {
    if ((perfexpert_list_item_t*)list->sentinel.next ==
        (perfexpert_list_item_t*)(&(list->sentinel))) {
        return NULL;
    }
    perfexpert_list_item_t* item = (perfexpert_list_item_t*)list->sentinel.next;
    return item;
}

/* perfexpert_list_get_last */
perfexpert_list_item_t* perfexpert_list_get_last(perfexpert_list_t* list) {
    perfexpert_list_item_t* item = (perfexpert_list_item_t*)list->sentinel.prev;
    return item;
}

/* perfexpert_list_get_size */
size_t perfexpert_list_get_size(perfexpert_list_t* list) {
    return list->length;
}

/* perfexpert_list_remove_item */
perfexpert_list_item_t *perfexpert_list_remove_item(perfexpert_list_t *list,
    perfexpert_list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->length--;

    return (perfexpert_list_item_t *)item->prev;
}

/* perfexpert_list_append */
void perfexpert_list_append(perfexpert_list_t *list,
    perfexpert_list_item_t *item) {
    perfexpert_list_item_t *sentinel = &(list->sentinel);
    item->prev = sentinel->prev;
    sentinel->prev->next = item;
    item->next = sentinel;
    sentinel->prev = item;
    list->length++;
}

/* perfexpert_list_prepend */
void perfexpert_list_prepend(perfexpert_list_t *list,
    perfexpert_list_item_t *item) {
    perfexpert_list_item_t *sentinel = &(list->sentinel);
    item->next = sentinel->next;
    item->prev = sentinel;
    sentinel->next->prev = item;
    sentinel->next = item;
    list->length++;
}

/* perfexpert_list_insert */
int perfexpert_list_insert(perfexpert_list_t *list,
    perfexpert_list_item_t *item, long long position) {
    volatile perfexpert_list_item_t *ptr, *next;
    int i = 0;

    if (position >= (long long)list->length) {
        return PERFEXPERT_ERROR;
    }

    if (0 == position) {
        perfexpert_list_prepend(list, item);
    } else {
        ptr = list->sentinel.next;
        for (i = 0; i < position - 1; i++) {
            ptr = ptr->next;
        }

        next = ptr->next;
        item->next = next;
        item->prev = ptr;
        next->prev = item;
        ptr->next  = item;
    }

    list->length++;

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_list_swap */
void perfexpert_list_swap(perfexpert_list_item_t *a,
    perfexpert_list_item_t *b) {
    volatile perfexpert_list_item_t *temp = NULL;

    /* Set the neighbor elements */
    a->prev->next = b;
    a->next->prev = b;
    b->prev->next = a;
    b->next->prev = a;

    /* Set the elements themselves */
    temp = a->prev;
    a->prev = b->prev;
    b->prev = temp;
    temp = a->next;
    a->next = b->next;
    b->next = temp;
}

/* perfexpert_list_move_before (move A before B) */
void perfexpert_list_move_before(perfexpert_list_item_t *a,
    perfexpert_list_item_t *b) {
    a->prev->next = a->next;
    a->next->prev = a->prev;
    a->prev = b->prev;
    a->next = b;
    b->prev->next = a;
    b->prev = a;
}

/* perfexpert_list_move_after (move A after B) */
void perfexpert_list_move_after(perfexpert_list_item_t *a,
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
