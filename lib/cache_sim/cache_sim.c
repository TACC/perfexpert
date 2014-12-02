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
#include <math.h>
#include <string.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_policy_lru.h"

/* List of policies */
static policy_t policies[] = {
    { "lru", &policy_lru_init, &policy_lru_put, &policy_lru_get },
    { NULL,  NULL,             NULL,            NULL }
};

/* cache_sim_init */
cache_handle_t* cache_sim_init(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity,
    const char *policy) {

    /* safety checks */
    if ((0 >= total_size) || (0 >= line_size) || (0 >= associativity)) {
        return NULL;
    }

    /* variables declaration and initialization */
    cache_handle_t *cache = NULL;

    #ifdef DEBUG
    printf("--------------------------------\n");
    #endif

    /* create the cache */
    if (NULL == (cache = cache_create(total_size, line_size, associativity))) {
        return NULL;
    }

    /* set the cache replacement policy (or algorithm) */
    if (0 != (set_policy(cache, policy))) {
        cache_destroy(cache);
        return NULL;
    }

    #ifdef DEBUG
    printf("Cache initialized successfully\n");
    printf("--------------------------------\n");
    #endif

    return cache;
}

/* create_cache */
static cache_handle_t* cache_create(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity) {
    /* variables declaration and initialization */
    cache_handle_t *cache = NULL;

    /* allocate memory for this cache */
    cache = (cache_handle_t *)malloc(sizeof(cache_handle_t));

    if (NULL == cache) {
        printf("unable to allocate memory for cache structure\n");
        return NULL;
    }

    /* initialize the cache parameters */
    cache->total_size    = total_size;
    cache->line_size     = line_size;
    cache->associativity = associativity;
    cache->total_lines   = total_size / line_size;
    cache->total_sets    = cache->total_lines / associativity;
    cache->offset_length = (int)log2(line_size);
    cache->set_length    = (int)log2(cache->total_sets);
    cache->put_fn        = NULL;
    cache->get_fn        = NULL;
    cache->data          = NULL;

    #ifdef DEBUG
    printf("Cache created successfully\n");
    printf("Cache size:      %d bytes\n", cache->total_size);
    printf("Line length:     %d bytes\n", cache->line_size);
    printf("Number of lines: %d\n", cache->total_lines);
    printf("Associativity:   %d\n", cache->associativity);
    printf("Number of sets:  %d\n", cache->total_sets);
    printf("Offset length:   %d bits\n", cache->offset_length);
    printf("Set length:      %d bits\n", cache->set_length);
    #endif

    return cache;
}

/* cache_destroy */
static void cache_destroy(cache_handle_t *cache) {
    if (NULL != cache) {
        if (NULL != cache->data) {
            free(cache->data);
        }
        free(cache);
    }
}

/* set_policy */
static int set_policy(cache_handle_t *cache, const char *policy) {
    int i = 0;

    /* find the requested policy and set the function */
    while (NULL != policies[i].name) {
        if (0 == strcmp(policy, policies[i].name)) {

            /* set the policy functions */
            cache->put_fn = policies[i].put_fn;
            cache->get_fn = policies[i].get_fn;

            #ifdef DEBUG
            printf("Replacem policy: %s\n", policy);
            #endif

            /* initialize the data area */
            policies[i].init_fn(cache);

            return 0;
        }
        i++;
    }

    printf("replacement policy not found\n");
    return 1;
}

// EOF
