/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
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

#ifndef CACHE_SIM_REUSE_H_
#define CACHE_SIM_REUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CACHE_SIM_H_
#include "cache_sim.h"
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

/* Struct of list item and base type for items that are put in lists */
typedef struct list_item list_item_t;
struct list_item {
    volatile list_item_t *next;
    volatile list_item_t *prev;
    uint64_t line_id;
    uint16_t age;
    uint16_t padding[3]; // can be safely used for something else
};

/* Struct of a list and base type for list containers */
typedef struct list {
    list_item_t head;
    volatile list_item_t *tmp;
    volatile uint32_t len;
} list_t;

/* Functions declaration */
static inline void list_prepend_item(list_t *list, list_item_t *item) {
    item->next = list->head.next;
    item->prev = &(list->head);
    list->head.next->prev = item;
    list->head.next = item;
    list->len++;
}

static inline void list_remove_item(list_t *list, list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->len--;
}

static inline void list_movetop_item(list_t *list, list_item_t *item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next = list->head.next;
    item->prev = &(list->head);
    list->head.next->prev = item;
    list->head.next = item;
}

#ifdef __cplusplus
}
#endif

#endif /* CACHE_SIM_REUSE_H_ */
