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

/* System standard headers */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_policy_lru.h"

/* policy_lru_init */
int policy_lru_init(cache_handle_t *cache) {
    /* variables declaration and initialization */
    void *ptr = NULL;

    /* allocate data area */
    ptr = malloc(sizeof(policy_lru_t) * cache->total_lines);

    if (NULL == ptr) {
        printf("unable to allocate memory for cache data\n");
        return CACHE_SIM_ERROR;
    }

    /* initialize data area */
    memset(ptr, UINT64_MAX, (sizeof(policy_lru_t) * cache->total_lines));
    cache->data = ptr;

    printf("Memory required: %d bytes\n", (sizeof(cache_handle_t) +
        (sizeof(policy_lru_t) * cache->total_lines)));

    return CACHE_SIM_SUCCESS;
}

/* policy_lru_read */
int policy_lru_access(cache_handle_t *cache, uint64_t set, uint64_t tag) {
    policy_lru_t *base_addr = NULL;
    policy_lru_t *lru = NULL;
    int way = 0;
    int i = 0;

    /* calculate data area base address for this set */
    base_addr = (policy_lru_t *)((uint64_t)cache->data +
        (set * cache->associativity * sizeof(policy_lru_t)));
    lru = base_addr;

    /* iterate across all the ways of the set */
    for (i = cache->associativity; i > 0; i--) {
        /* check if data is present */
        if (base_addr->tag == tag) {
            /* update access age */
            base_addr->age = cache->access;
            #ifdef DEBUG
            printf("HIT    set [%2d:%d]\n", set, i);
            #endif
            return CACHE_SIM_L1_HIT;
        }

        /* if it is a free way use it (bonus!) */
        if (UINT32_MAX == base_addr->age) {
            lru = base_addr;
            way = i;
            break;
        }

        /* find the last recently used way in the set */
        if (base_addr->age <= lru->age) {
            lru = base_addr;
            way = i;
        }

        /* increment way address */
        base_addr++;
    }

    /* if data was not found, load it */
    lru->age = cache->access;
    lru->tag = tag;

    #ifdef DEBUG
    printf("LOAD   set [%2d:%d]\n", set, way);
    #endif

    return CACHE_SIM_L1_MISS;
}

// EOF
