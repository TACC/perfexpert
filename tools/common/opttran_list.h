/*
 * Copyright (c) 2013      University of Texas at Austin. All rights reserved.
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
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

/**
 * @file 
 *
 * The opttran_list interface is used to provide a generic doubly-linked list
 * container for OptTran tools. It was inspired by (but is slightly different
 * than) the Open MPI's lists interface, which was inspired by Stantard Template
 * Library (STL) std::list class. One notable difference Open MPI's is that is
 * that here, we provide a small subset of functions. The main difference
 * between Open MPI's lists interface and std::list classe is that when an
 * opttran_list_t is destroyed, all of the opttran_list_item_t objects that it
 * contains are orphaned -- they are not destroyed.
 *
 * The general idea is that opttran_list_item_t can be put on an opttran_list_t.
 * Hence, you create a new type that derives from opttran_list_item_t; this new
 * type can then be used with opttran_list_t containers.
 *
 * NOTE: opttran_list_item_t instances can only be on one list at a time.
 * Specifically, if you add an opttran_list_item_t to one list, and then add it
 * to another list (without first removing it from the first list), you will
 * effectively be hosing the first list. You have been warned.
 *
 * This list interface does not implement locks. Thus it's is possible the
 * value returned by the opttran_list_get_size is incorrect.
 */

#ifndef OPTTRAN_LIST_H_
#define OPTTRAN_LIST_H_

#ifndef OPTTRAN_CONSTANTS_H
#include "opttran_constants.h"
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

/**
 * Structure of list item and base type for items that are put in list 
 * containers.
 */
typedef struct opttran_list_item opttran_list_item_t;
struct opttran_list_item {
    /** Pointer to next list item     */
    volatile opttran_list_item_t *next;
    /** Pointer to previous list item */
    volatile opttran_list_item_t *prev;
};

/**
 * Struct of a list and base type for list containers.
 */
typedef struct opttran_list opttran_list_t;
struct opttran_list {
    /** head and tail item of the list                     */
    opttran_list_item_t sentinel;
    /** quick reference to the number of items in the list */
    volatile size_t length;
};

static inline void opttran_list_construct(opttran_list_t *list) {
    list->sentinel.next = &list->sentinel;
    list->sentinel.prev = &list->sentinel;
    list->length = 0;
}

static inline void opttran_list_destruct(opttran_list_t *list) {
    opttran_list_construct(list);
}

static inline void opttran_list_item_construct(opttran_list_item_t *item) {
    item->next = item->prev = NULL;
}

static inline void opttran_list_item_destruct(opttran_list_item_t *item) {
    /* nothing to do */
}

/**
 * Get the next item in a list.
 *
 * @param[in] item   Current list item
 * @retval    item_t The next item in the list
 */
#define opttran_list_get_next(item) ((item) ? \
    ((opttran_list_item_t*) ((opttran_list_item_t*)(item))->next) : NULL)

/**
 * Get the previous item in a list.
 *
 * @param[in] item   Current list item
 * @retval    item_t The previous item in the list
 */
#define opttran_list_get_prev(item) ((item) ? \
    ((opttran_list_item_t*) ((opttran_list_item_t*)(item))->prev) : NULL)

/**
 * Check for empty list
 *
 * @param[in] list The  List container
 * @retval    OPTTRAN_TRUE  If list's size is 0
 * @retval    OPTTRAN_FALSE Otherwise
 */
static inline int opttran_list_is_empty(opttran_list_t* list) {
    return (list->sentinel.next == &(list->sentinel) ?
            OPTTRAN_FALSE : OPTTRAN_TRUE);
}

/**
 * Return the first item on the list (does not remove it).
 *
 * @param[in] list   The list container
 * @retval    item_t A pointer to the first item on the list
 */
static inline opttran_list_item_t* opttran_list_get_first(opttran_list_t* list) {
    opttran_list_item_t* item = (opttran_list_item_t*)list->sentinel.next;
    return item;
}

/**
 * Return the number of items in a list
 *
 * @param[in] list   The list container
 * @retval    size_t The size of the list
 */
static inline size_t opttran_list_get_size(opttran_list_t* list) {
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
static inline opttran_list_item_t *opttran_list_remove_item(opttran_list_t *list,
                                                    opttran_list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->length--;
    
    return (opttran_list_item_t *)item->prev;
}

/**
 * Append an item to the end of the list.
 *
 * @param[in] list The list container
 * @param[in] item The item to append
 */
static inline void opttran_list_append(opttran_list_t *list,
                                       opttran_list_item_t *item) {
    opttran_list_item_t *sentinel = &(list->sentinel);

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

#endif /* OPTTRAN_LIST_H */