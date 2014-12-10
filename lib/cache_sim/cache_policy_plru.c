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
#include <math.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_policy_plru.h"

/* policy_plru_init */
int policy_plru_init(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* variables declaration and initialization */
    uint64_t *plru_mask = NULL;
    policy_plru_t *block = NULL;

    /* allocate data area */
    cache->data = malloc((sizeof(policy_plru_t) * cache->total_lines) +
        ((cache->total_sets * sizeof(uint64_t))));

    if (NULL == cache->data) {
        printf("Error: unable to allocate memory for cache data\n");
        return CACHE_SIM_ERROR;
    }

    /* initialize data area: PLRU masks */
    for (plru_mask = (uint64_t *)cache->data;
        (uint64_t)plru_mask < ((uint64_t)cache->data +
            (cache->total_sets * sizeof(uint64_t)));
        plru_mask++) {
        *plru_mask = 0;
    }

    /* initialize data area: blocks on sets */
    for (block = (policy_plru_t *)((uint64_t)cache->data +
            (cache->total_sets * sizeof(uint64_t)));
        (uint64_t)block < ((uint64_t)cache->data +
            (cache->total_sets * sizeof(uint64_t)) +
            (sizeof(policy_plru_t) * cache->total_lines));
        block++) {
        /* write UINT64_MAX in all blocks... */
        block->line_id = UINT64_MAX;
        block->load = LOAD_ACCESS;
    }

    /* print out how much memory it requires */
    printf("Memory required: %9d bytes\n",
        ((sizeof(policy_plru_t) * cache->total_lines) +
        (cache->total_sets * sizeof(uint64_t))));

    return CACHE_SIM_SUCCESS;
}

/* policy_plru_access */
int policy_plru_access(cache_handle_t *cache, const uint64_t line_id,
    const load_t load) {
    static policy_plru_t *way_addr = NULL;
    static uint64_t *plru_mask = NULL;
    static uint64_t set = UINT64_MAX;
    static int way = 0, i = 0;
    int bit = 0, bit_set = 0, bit_offset = 0;

    /* calculate set for this address */
    set = line_id;
    CACHE_SIM_LINE_ID_TO_SET(set);

    /* calculate the base address for the PLRU mask of this set */
    plru_mask = (uint64_t *)((uint64_t)cache->data + (set * sizeof(uint64_t)));

    /* calculate the base address of the first way on the set */
    way_addr = (policy_plru_t *)((uint64_t)cache->data +
        (cache->total_sets * sizeof(uint64_t)) +
        (set * cache->associativity * sizeof(policy_plru_t)));

    /* iterate across all the ways of the set */
    for (way = 0; way < cache->associativity; way++, way_addr++) {
        /* check if data is present */
        if (line_id == way_addr->line_id) {
            #ifdef DEBUG
            printf("HIT    line id [%018p] set [%2d:%d]\n", line_id, set, way);
            #endif

            /* update PLRU mask by inverting the bits used by this way */
            for (i = 0; i < (int)log2(cache->associativity); i++) {
                /* find out which bit should be toggled */
                bit = bit_set + bit_offset;

                /* toggle it */
                TOGGLE_BIT(*plru_mask, bit);

                /* add the bit offset */
                bit_offset = way >> ((int)log2(cache->associativity) - 1 - i);

                /* move to the next set of bits */
                bit_set <<= 1;
                bit_set++;
            }

            /* if the hit was on a prefetched line */
            if (LOAD_PREFETCH == way_addr->load) {
                /* reset load reason */
                way_addr->load = LOAD_ACCESS;

                return (CACHE_SIM_L1_HIT + CACHE_SIM_L1_HIT_PREFETCH);
            }

            return CACHE_SIM_L1_HIT;
        }
    }

    /* find the PLRU way and update PRLU mask */
    for (i = 0, way = 0; i < (int)log2(cache->associativity); i++) {
        /* find out which bit should be read */
        bit = bit_set + bit_offset;

        /* read it */
        if (BIT_IS_SET(*plru_mask, bit)) {
            way++;
        }

        /* toggle it */
        TOGGLE_BIT(*plru_mask, bit);

        /* jump over the next set of bits (0, 1, 3, 7, 15, ...) */
        bit_set <<= 1;
        bit_set++;

        /* find the right bit on the set (depends on the previous bits read) */
        bit_offset = way;

        /* save the value of the last bit read */
        way <<= 1;
    }
    /* correct the way value since the last shift left was unnecessary */
    way >>= 1;

    /* calculate data area base address for this way */
    way_addr = (policy_plru_t *)((uint64_t)cache->data +
        (cache->total_sets * sizeof(uint64_t)) +
        (set * cache->associativity * sizeof(policy_plru_t)) +
        (way * sizeof(policy_plru_t)));

    /* report a miss and load the line */
    #ifdef DEBUG
    printf("MISS   line id [%018p]\n", line_id);
    printf("LOAD   line id [%018p] set [%2d:%d] load reason [%d]\n", line_id,
        set, way, load);
    #endif

    /* if the evicted line was prefetched and never accessed */
    if (LOAD_PREFETCH == way_addr->load) {
        /* load the data */
        way_addr->line_id = line_id;
        way_addr->load = load;

        return (CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT);
    }

    /* load the data */
    way_addr->line_id = line_id;
    way_addr->load = load;

    return CACHE_SIM_L1_MISS;
}

// EOF
