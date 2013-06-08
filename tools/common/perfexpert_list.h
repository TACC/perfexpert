/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
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
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

/**
 * @file 
 *
 * The perfexpert_list interface is used to provide a generic doubly-linked list
 * container for PerfExpert tools. It was inspired by (but is slightly different
 * than) the Open MPI's lists interface, which was inspired by Stantard Template
 * Library (STL) std::list class. One notable difference Open MPI's is that is
 * that here, we provide a small subset of functions. The main difference
 * between Open MPI's lists interface and std::list classe is that when an
 * perfexpert_list_t is destroyed, all of the perfexpert_list_item_t objects
 * that it contains are orphaned -- they are not destroyed.
 *
 * The general idea is that perfexpert_list_item_t can be put on an
 * perfexpert_list_t. Hence, you create a new type that derives from
 * perfexpert_list_item_t; this new type can then be used with perfexpert_list_t
 * containers.
 *
 * NOTE: perfexpert_list_item_t instances can only be on one list at a time.
 * Specifically, if you add an perfexpert_list_item_t to one list, and then add
 * it to another list (without first removing it from the first list), you will
 * effectively be hosing the first list. You have been warned.
 *
 * This list interface does not implement locks. Thus it's is possible the
 * value returned by the perfexpert_list_get_size is incorrect.
 */

#ifndef PERFEXPERT_LIST_H_
#define PERFEXPERT_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef PERFEXPERT_CONSTANTS_H_
#include "perfexpert_constants.h"
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

/**
 * Structure of list item and base type for items that are put in list 
 * containers.
 */
typedef struct perfexpert_list_item perfexpert_list_item_t;
struct perfexpert_list_item {
    /** Pointer to next list item     */
    volatile perfexpert_list_item_t *next;
    /** Pointer to previous list item */
    volatile perfexpert_list_item_t *prev;
};

/**
 * Struct of a list and base type for list containers.
 */
typedef struct perfexpert_list perfexpert_list_t;
struct perfexpert_list {
    /** head and tail item of the list                     */
    perfexpert_list_item_t sentinel;
    /** quick reference to the number of items in the list */
    volatile size_t length;
};

static inline void perfexpert_list_construct(perfexpert_list_t *list) {
    list->sentinel.next = &list->sentinel;
    list->sentinel.prev = &list->sentinel;
    list->length = 0;
}

static inline void perfexpert_list_destruct(perfexpert_list_t *list) {
    perfexpert_list_construct(list);
}

static inline void perfexpert_list_item_construct(perfexpert_list_item_t *item) {
    item->next = item->prev = NULL;
}

static inline void perfexpert_list_item_destruct(perfexpert_list_item_t *item) {
    /* nothing to do */
}

/**
 * Get the next item in a list.
 *
 * @param[in] item   Current list item
 * @retval    item_t The next item in the list
 */
#define perfexpert_list_get_next(item) ((item) ? \
    ((perfexpert_list_item_t*) ((perfexpert_list_item_t*)(item))->next) : NULL)

/**
 * Get the previous item in a list.
 *
 * @param[in] item   Current list item
 * @retval    item_t The previous item in the list
 */
#define perfexpert_list_get_prev(item) ((item) ? \
    ((perfexpert_list_item_t*) ((perfexpert_list_item_t*)(item))->prev) : NULL)

/**
 * Check for empty list
 *
 * @param[in] list The  List container
 * @retval    PERFEXPERT_TRUE  If list's size is 0
 * @retval    PERFEXPERT_FALSE Otherwise
 */
static inline int perfexpert_list_is_empty(perfexpert_list_t* list) {
    return (list->sentinel.next == &(list->sentinel) ?
            PERFEXPERT_FALSE : PERFEXPERT_TRUE);
}

/**
 * Return the first item on the list (does not remove it).
 *
 * @param[in] list   The list container
 * @retval    item_t A pointer to the first item on the list
 */
static inline perfexpert_list_item_t* perfexpert_list_get_first(perfexpert_list_t* list) {
    perfexpert_list_item_t* item = (perfexpert_list_item_t*)list->sentinel.next;
    return item;
}

/**
 * Return the number of items in a list
 *
 * @param[in] list   The list container
 * @retval    size_t The size of the list
 */
static inline size_t perfexpert_list_get_size(perfexpert_list_t* list) {
    return list->length;
}

/**
 * Remove an item from a list.
 *
 * @param[in] list   The list container
 * @param[in] item   The item to remove
 * @retval    item_t A pointer to the item on the list previous to the one that
 *                   was removed.
 */
static inline perfexpert_list_item_t *perfexpert_list_remove_item(perfexpert_list_t *list,
                                                                  perfexpert_list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->length--;
    
    return (perfexpert_list_item_t *)item->prev;
}

/**
 * Append an item to the end of the list.
 *
 * @param[in] list The list container
 * @param[in] item The item to append
 */
static inline void perfexpert_list_append(perfexpert_list_t *list,
                                          perfexpert_list_item_t *item) {
    perfexpert_list_item_t *sentinel = &(list->sentinel);

    /* set new element's previous pointer             */
    item->prev = sentinel->prev;
    /* reset previous pointer on current last element */
    sentinel->prev->next = item;
    /* reset new element's next pointer               */
    item->next = sentinel;
    /* reset the list's tail element previous pointer */
    sentinel->prev = item;
    /* increment list element counter */
    list->length++;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_LIST_H */