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
#include "cache_sim_types.h"
#include "cache_sim_conflict.h"
#include "cache_sim_reuse.h"

/* cache_sim_conflict_enable */
int cache_sim_conflict_enable(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* reset conflicts counters */
    cache->conflict = 0;

    /* sanity check: reuse is already enabled with the minimum limit */
    if ((NULL != cache->reuse_data) &&
        ((cache->total_lines + 1) < cache->reuse_limit)) {
        return CACHE_SIM_SUCCESS;
    }

    /* enable conflict if reuse distance is off */
    if (NULL == cache->reuse_data) {
        if (CACHE_SIM_SUCCESS !=
            cache_sim_reuse_enable(cache, (cache->total_lines + 1))) {
            printf("Error: unable to enable set associative conflict\n");
            return CACHE_SIM_ERROR;
        }
    }
    /* enabled conflict if reuse limit is too short */
    else if ((cache->total_lines + 1) > cache->reuse_limit) {
        printf("Warning: reuse distance was already enabled, conflict analysis "
            "will increase the reuse limit from %16"PRIu64" to %16"PRIu64"\n",
            cache->reuse_limit, (cache->total_lines + 1));

        /* disable cache reuse */
        if (CACHE_SIM_SUCCESS != cache_sim_reuse_disable(cache)) {
            printf("Error: unable to disable cache reuse\n");
            return CACHE_SIM_ERROR;
        }

        /* enable cache reuse using the new limit */
        if (CACHE_SIM_SUCCESS !=
            cache_sim_reuse_enable(cache, (cache->total_lines + 1))) {
            printf("Error: unable to enable cache reuse limit\n");
            return CACHE_SIM_ERROR;
        }
    }

    /* be nice and print something... */
    printf("--------------------------------\n");
    printf(" Set associative conflict is ON \n");
    printf("--------------------------------\n");

    return CACHE_SIM_SUCCESS;
}

// EOF
