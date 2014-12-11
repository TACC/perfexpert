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
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_sim_types.h"
#include "cache_sim_util.h"
#include "cache_sim_reuse.h"

/* cache_sim_access */
int cache_sim_access(cache_handle_t *cache, const uint64_t address) {
    /* variables declaration */
    static int rc = CACHE_SIM_ERROR;
    static uint64_t line_id = UINT64_MAX;

    /* increment access counter */
    cache->access++;

    /* calculate tag and set for this address */
    line_id = address;
    CACHE_SIM_ADDRESS_TO_LINE_ID(line_id);

    #ifdef DEBUG
    printf("ACCESS address [%018p]\n", address);
    #endif

    /* call the replacement algorithm access function and evaluate the result */
    switch (rc = cache->access_fn(cache, line_id, LOAD_ACCESS)) {
        /* hit on a prefetched line */
        case CACHE_SIM_L1_HIT + CACHE_SIM_L1_HIT_PREFETCH: // fallover!
            /* increment prefetcher hits counter */
            cache->prefetcher_hit++;

            /* if the next line prefetcher is tagged, keep prefething */
            if (PREFETCHER_NEXT_LINE_TAGGED == cache->next_line) {
                /* if the next line load evicts a prefetched line */
                if ((CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT) ==
                    cache->access_fn(cache, (line_id + cache->line_size),
                        LOAD_PREFETCH)) {
                    /* increment prefetched evicted lines counter */
                    cache->prefetcher_evict++;
                }

                /* increment prefetcher next line counter */
                cache->prefetcher_next_line++;

                #ifdef DEBUG
                printf("PREFET line id [%018p]\n", line_id + cache->line_size);
                #endif
            }

        /* hit on a pre-loaded cache line */
        case CACHE_SIM_L1_HIT:
            /* increment hits counter */
            cache->hit++;
            break;
        /* miss on a prefetched line that have never been accessed */
        case CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT: // fallover!
            /* increment prefetched evicted lines counter */
            cache->prefetcher_evict++;
        /* common miss */
        case CACHE_SIM_L1_MISS:
            /* increment misses counter */
            cache->miss++;

            /* if prefetcher next line is ON fetch next line */
            if ((PREFETCHER_NEXT_LINE_SINGLE == cache->next_line) ||
               (PREFETCHER_NEXT_LINE_TAGGED == cache->next_line)) {
                /* if the next line load evicts a prefetched line */
                if ((CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT) ==
                    cache->access_fn(cache, (line_id + cache->line_size),
                        LOAD_PREFETCH)) {
                    /* increment prefetched evicted lines counter */
                    cache->prefetcher_evict++;
                }

                /* increment prefetcher next line counter */
                cache->prefetcher_next_line++;

                #ifdef DEBUG
                printf("PREFET line id [%018p]\n", line_id + cache->line_size);
                #endif
            }
            break;
    }

    /* calculate reuse distance and check for set associative conflicts */
    if (NULL != cache->reuse_data) {
        /* check for set associativity conflicts */
        if ((cache->total_lines > cache_sim_reuse_get_age(cache, line_id)) &&
            (CACHE_SIM_L1_MISS == rc)) {

            /* increment conflicts counter */
            cache->conflict++;

            #ifdef DEBUG
            printf("CONFLI line id [%018p] distance [%"PRIu64"]\n", line_id,
                cache_sim_reuse_get_age(cache, line_id));
            #endif

            rc += CACHE_SIM_L1_MISS_CONFLICT;
        }

        /* calculate reuse distance */
        cache->reuse_fn(cache, line_id);
    }

    return rc;
}

// EOF
