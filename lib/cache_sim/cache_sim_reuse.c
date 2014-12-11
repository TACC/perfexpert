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
#include <stdlib.h>
#include <stdio.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_sim_util.h"
#include "cache_sim_reuse.h"

/* cache_sim_reuse_enable */
int cache_sim_reuse_enable(cache_handle_t *cache, const uint64_t limit) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* sanity check: already enabled with the same limit */
    if ((NULL != cache->reuse_data) && (limit == cache->reuse_limit)) {
        printf("Warning: reuse distance was already enabled\n");
        return CACHE_SIM_SUCCESS;
    }

    /* sanity check: already enabled with a different limit */
    if ((NULL != cache->reuse_data) && (limit != cache->reuse_limit)) {
        printf("Warning: reuse distance was already enabled with a different "
            "limit\n");
        if (CACHE_SIM_SUCCESS != cache_sim_reuse_disable(cache)) {
            printf("Error: unable to redefine cache reuse limit\n");
            return CACHE_SIM_ERROR;
        }
    }

    /* sanity check: limit higher than zero */
    if (0 >= limit) {
        printf("Error: limit is smaller than 1\n");
        return CACHE_SIM_ERROR;
    }

    /* variables declaration */
    list_item_reuse_t *item = NULL;
    list_t *list = NULL;
    int i = 0;

    /* set reuse limit */
    cache->reuse_limit = limit;

    /* allocate memory for reuse data area acording to reuse limit */
    if (0 == cache->reuse_limit) {
        /* for unlimited reuse distance */
        cache->reuse_data = malloc(sizeof(list_t));
        if (NULL == cache->reuse_data) {
            printf("unable to allocate memory to calculate reuse distance\n");
            return CACHE_SIM_ERROR;
        }

        /* initialize the list */
        list = cache->reuse_data;
        list->head.next = &(list->head);
        list->head.prev = &(list->head);
        list->len = 0;

        /* set the reuse distance function */
        cache->reuse_fn = &cache_sim_reuse_unlimited;

        printf("--------------------------------\n");
        printf("      Reuse distance is ON      \n");
        printf("Reuse limit:           unlimited\n");
        printf("Memory required: %d bytes +%d/l\n", sizeof(list_t),
            sizeof(list_item_t));
        printf("--------------------------------\n");
    } else {
        /* for a particular reuse distance limit */
        cache->reuse_data = malloc(sizeof(list_t) +
            (cache->reuse_limit * sizeof(list_item_reuse_t)));
        if (NULL == cache->reuse_data) {
            printf("Error: unable to allocate memory to reuse distance area\n");
            return CACHE_SIM_ERROR;
        }

        /* initialize the list */
        list = cache->reuse_data;
        list->head.next = &(list->head);
        list->head.prev = &(list->head);
        list->len = 0;

        /* set the reuse distance function */
        cache->reuse_fn = &cache_sim_reuse_limited;

        /* initialize list elements */
        item = (list_item_reuse_t *)((uint64_t)cache->reuse_data + sizeof(list_t));
        for (i = 0; i < cache->reuse_limit; i++) {
            list_prepend_item((list_t *)(cache->reuse_data), (list_item_t *)item);
            item->line_id = UINT64_MAX;
            item->age = UINT16_MAX;
            item++;
        }

        printf("--------------------------------\n");
        printf("      Reuse distance is ON      \n");
        printf("Reuse limit:     %15"PRIu64"\n", cache->reuse_limit);
        printf("Memory required: %9d bytes\n", sizeof(list_t) +
            (cache->reuse_limit * sizeof(list_item_reuse_t)));
        printf("--------------------------------\n");
    }

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_reuse_disable */
int cache_sim_reuse_disable(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* sanity check: already disabled */
    if (NULL == cache->reuse_data) {
        printf("Error: reuse distance is not enabled\n");
        return CACHE_SIM_ERROR;
    }

    /* variables declaration */
    // list_item_t *item;

    printf("--------------------------------\n");
    // for (item = (list_item_t *)((list_t *)cache->reuse_data)->head.next;
    //     item != &(((list_t *)cache->reuse_data)->head);
    //     item = (list_item_t *)item->next) {
    //     printf("Line: %018p, Distance: %"PRIu64"\n", item->line_id, item->age);
    // }
    printf(" (print something nice here...) \n");
    printf("      Reuse distance is OFF     \n");
    printf("--------------------------------\n");

    // TODO: free the list elements when reuse distance is unlimited
    free(cache->reuse_data);

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_reuse_limited */
int cache_sim_reuse_limited(cache_handle_t *cache, const uint64_t line_id) {
    /* variable declarations */
    static list_item_reuse_t *item = NULL;

    /* for all elements in the list of lines... */
    for (item = (list_item_reuse_t *)((list_t *)cache->reuse_data)->head.next;
        (void *)item != (void *)&(((list_t *)cache->reuse_data)->head);
        item = (list_item_reuse_t *)item->next) {
        /* ...if we find the line... */
        if (line_id == item->line_id) {
            /* ...report it... */
            #ifdef DEBUG
            printf("REUSE  line id [%018p] reuse distance [%"PRIu64"]\n",
                item->line_id, item->age);
            #endif

            /* ...move the line to the top of the list... */
            list_movetop_item((list_t *)cache->reuse_data, (list_item_t *)item);

            /* ...set the line's age to zero */
            item->age = 0;

            return CACHE_SIM_SUCCESS;
        }

        /* increment age for every line above the line we are looking for */
        item->age++;
    }

    /* if the line was not found take the last one and move it up to the top */
    item = (list_item_reuse_t *)((list_t *)cache->reuse_data)->head.prev;
    list_movetop_item((list_t *)cache->reuse_data, (list_item_t *)item);
    item->line_id = line_id;
    item->age = 0;

    #ifdef DEBUG
    printf("USE    line id [%018p]\n", item->line_id);
    #endif

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_reuse_unlimited */
int cache_sim_reuse_unlimited(cache_handle_t *cache, const uint64_t line_id) {
    /* variables declaration */
    static list_item_reuse_t *item = NULL;

    /* for all elements in the list of lines... */
    for (item = (list_item_reuse_t *)((list_t *)cache->reuse_data)->head.next;
        (void *)item != (void *)&(((list_t *)cache->reuse_data)->head);
        item = (list_item_reuse_t *)item->next) {
        /* ...if we find the line... */
        if (line_id == item->line_id) {
            /* ...report it... */
            #ifdef DEBUG
            printf("REUSE  line id [%018p] reuse distance [%"PRIu64"]\n",
                item->line_id, item->age);
            #endif

            /* ...move the line to the top of the list... */
            list_movetop_item((list_t *)cache->reuse_data, (list_item_t *)item);

            /* ...set the line's age to zero */
            item->age = 0;

            return CACHE_SIM_SUCCESS;
        }

        /* increment age for every line above the line we are looking for */
        item->age++;
    }

    /* if the line was not found, add it to the list */
    item = malloc(sizeof(list_item_reuse_t));
    if (NULL == item) {
        return CACHE_SIM_ERROR;
    }
    list_prepend_item((list_t *)cache->reuse_data, (list_item_t *)item);
    item->line_id = line_id;
    item->age = 0;

    #ifdef DEBUG
    printf("USE    line id [%018p]\n", item->line_id);
    #endif

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_reuse_get_age */
uint64_t cache_sim_reuse_get_age(cache_handle_t *cache, const uint64_t line_id) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* variable declarations */
    static list_item_reuse_t *item = NULL;

    /* for all elements in the list of lines... */
    for (item = (list_item_reuse_t *)((list_t *)cache->reuse_data)->head.next;
        (void *)item != (void *)&(((list_t *)cache->reuse_data)->head);
        item = (list_item_reuse_t *)item->next) {
        /* ...if we find the line... */
        if (line_id == item->line_id) {
            /* ...report its age... */
            return item->age;
        }
    }

    return UINT64_MAX;
}

// EOF
