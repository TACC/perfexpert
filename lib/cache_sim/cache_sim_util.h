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

#ifndef CACHE_SIM_UTIL_H_
#define CACHE_SIM_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

#ifndef CACHE_SIM_H_
#include "cache_sim.h"
#endif

/* Macro to extract the ID (tag - offset), offset, set, and tag of an address */
#ifndef CACHE_SIM_ADDRESS_TO_LINE_ID
#define CACHE_SIM_ADDRESS_TO_LINE_ID(a) \
    (a >>= cache->offset_length); \
    (a <<= cache->offset_length);
#endif

#ifndef CACHE_SIM_ADDRESS_TO_OFFSET
#define CACHE_SIM_ADDRESS_TO_OFFSET(a) \
    (a <<= ((sizeof(uint64_t)*8) - cache->offset_length)); \
    (a >>= ((sizeof(uint64_t)*8) - cache->offset_length));
#endif

#ifndef CACHE_SIM_ADDRESS_TO_SET
#define CACHE_SIM_ADDRESS_TO_SET(a) \
    (a <<= ((sizeof(uint64_t)*8) - cache->set_length - cache->offset_length)); \
    (a >>= ((sizeof(uint64_t)*8) - cache->set_length));
#endif

#ifndef CACHE_SIM_ADDRESS_TO_TAG
#define CACHE_SIM_ADDRESS_TO_TAG(a) \
    (a >>= (cache->set_length + cache->offset_length));
#endif

#ifndef CACHE_SIM_LINE_ID_TO_SET
#define CACHE_SIM_LINE_ID_TO_SET(a) CACHE_SIM_ADDRESS_TO_SET(a)
#endif

/* Functions declaration: list manipulation */
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

#endif /* CACHE_SIM_UTIL_H_ */
