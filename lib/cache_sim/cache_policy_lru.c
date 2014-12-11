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
#include "cache_sim_types.h"
#include "cache_sim_util.h"
#include "cache_policy_lru.h"

/* policy_lru_init */
int policy_lru_init(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* variables declaration and initialization */
    void *ptr = NULL;
    uint64_t i = 0;

    /* allocate data area */
    ptr = malloc(sizeof(policy_lru_t) * cache->total_lines);

    if (NULL == ptr) {
        printf("Error: unable to allocate memory for cache data\n");
        return CACHE_SIM_ERROR;
    }
    cache->data = ptr;

    /* initialize data area */
    for (i = (uint64_t)cache->data;
        i < ((uint64_t)cache->data + (sizeof(policy_lru_t) * cache->total_lines));
        i += sizeof(uint64_t)) {
        /* write UINT64_MAX everywhere... */
        *(uint64_t *)i = UINT64_MAX;
    }

    printf("Memory required: %9d bytes\n", (sizeof(cache_handle_t) +
        (sizeof(policy_lru_t) * cache->total_lines)));

    return CACHE_SIM_SUCCESS;
}

/* policy_lru_read */
int policy_lru_access(cache_handle_t *cache, const uint64_t line_id,
    const load_t load) {
    static policy_lru_t *base_addr = NULL;
    static policy_lru_t *lru = NULL;
    static uint64_t set = UINT64_MAX;
    static int way = 0;
    register int i = 0;

    /* calculate tag and set for this address */
    set = line_id;
    CACHE_SIM_LINE_ID_TO_SET(set);

    /* calculate data area base address for this set */
    base_addr = (policy_lru_t *)((uint64_t)cache->data +
        (set * cache->associativity * sizeof(policy_lru_t)));
    lru = base_addr;

    /* iterate across all the ways of the set */
    for (i = cache->associativity; i > 0; i--) {
        /* check if data is present */
        if (base_addr->line_id == line_id) {
            /* update access age */
            base_addr->age = cache->access;

            #ifdef DEBUG
            printf("HIT    line id [%018p] set [%2d:%d]\n", line_id, set, i);
            #endif

            /* if the hit was on a prefetched line */
            if (LOAD_PREFETCH == base_addr->load) {
                /* update the load reason */
                base_addr->load = LOAD_ACCESS;

                return (CACHE_SIM_L1_HIT + CACHE_SIM_L1_HIT_PREFETCH);
            }

            return CACHE_SIM_L1_HIT;
        }

        /* if it is a free way use it (bonus!) */
        if (UINT64_MAX == base_addr->line_id) {
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

    /* if data was not found, report that and load it */
    #ifdef DEBUG
    printf("MISS   line id [%018p]\n", line_id);
    printf("LOAD   line id [%018p] set [%2d:%d] load reason [%d]\n", line_id,
        set, way, load);
    #endif

    /* if the evicted line was prefetched and never accessed */
    if (LOAD_PREFETCH == lru->load) {
        /* load the data */
        lru->age = cache->access;
        lru->line_id = line_id;
        lru->load = load;

        return (CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT);
    }

    /* load the data */
    lru->age = cache->access;
    lru->line_id = line_id;
    lru->load = load;

    return CACHE_SIM_L1_MISS;
}

// EOF
