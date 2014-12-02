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
    ptr = malloc((sizeof(uint64_t) + sizeof(uint8_t)) * cache->total_lines);

    if (NULL == ptr) {
        printf("unable to allocate memory for cache data\n");
        return 1;
    }

    /* initialize data area */
    memset(ptr, 0, ((sizeof(uint64_t) + sizeof(uint8_t)) * cache->total_lines));
    cache->data = ptr;

    #ifdef DEBUG
    printf("Memory required: %d bytes\n", (sizeof(cache_handle_t) +
        ((sizeof(uint64_t) + sizeof(uint8_t)) * cache->total_lines)));
    #endif

    return 0;
}

/* policy_lru_put */
int policy_lru_put(cache_handle_t *cache, uint64_t address) {
    uint32_t set = address;
    uint32_t tag = address;

    /* calculate set and tag (offset does not matter) */
    POLICY_LRU_SET(set);
    POLICY_LRU_TAG(tag);

    #ifdef DEBUG
    printf("Address: [%d], Tag: [%d], Set: [%d]\n", address, tag, set);
    #endif

    // TODO: write the TAG to a specific SET and update the alrogithm index

}

/* policy_lru_get */
int policy_lru_get(cache_handle_t *cache, uint64_t address) {
    // TODO: everything :/
}

// EOF
