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

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_sim_prefetcher.h"

/* cache_sim_prefetcher_enable */
int cache_sim_prefetcher_enable(cache_handle_t *cache, prefetcher_t type) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* enable prefetcher next line on a miss (single line prefetching) */
    if (PREFETCHER_NEXT_LINE_SINGLE == type) {
        cache->next_line = PREFETCHER_NEXT_LINE_SINGLE;
    }

    /* enable prefetcher next line on a miss (single line prefetching) */
    if (PREFETCHER_NEXT_LINE_TAGGED == type) {
        cache->next_line = PREFETCHER_NEXT_LINE_TAGGED;
    }

    /* unknown prefetcher type */
    if (PREFETCHER_INVALID <= type) {
        printf("Error: unknown prefetcher type\n");
        return CACHE_SIM_ERROR;
    }

    /* be nice and print something... */
    printf("--------------------------------\n");
    printf("Next line prefetcher: %10s\n",
        (PREFETCHER_INVALID == cache->next_line) ? "OFF" :
            ((PREFETCHER_NEXT_LINE_SINGLE == cache->next_line) ? "SINGLE" :
                (PREFETCHER_NEXT_LINE_TAGGED == cache->next_line) ? "TAGGED" :
                    "UNKNOWN"));
    printf("      Hardware prefetcher       \n");
    printf("--------------------------------\n");

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_prefetcher_disable */
int cache_sim_prefetcher_disable(cache_handle_t *cache, prefetcher_t type) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* disable prefetcher next line */
    if ((PREFETCHER_NEXT_LINE_SINGLE == type) ||
        (PREFETCHER_NEXT_LINE_TAGGED == type)) {
        cache->next_line = PREFETCHER_INVALID;
    }

    /* unknown prefetcher type */
    if (PREFETCHER_INVALID <= type) {
        printf("Error: unknown prefetcher type\n");
        return CACHE_SIM_ERROR;
    }

    /* be nice and print something... */
    printf("--------------------------------\n");
    printf("Next line prefetcher: %10s\n",
        (PREFETCHER_INVALID == cache->next_line) ? "OFF" :
            ((PREFETCHER_NEXT_LINE_SINGLE == cache->next_line) ? "SINGLE" :
                (PREFETCHER_NEXT_LINE_TAGGED == cache->next_line) ? "TAGGED" :
                    "UNKNOWN"));
    printf("      Hardware prefetcher       \n");
    printf("--------------------------------\n");

    return CACHE_SIM_SUCCESS;
}

// EOF
