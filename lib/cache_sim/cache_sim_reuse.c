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

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_sim_reuse.h"

/* cache_sim_reuse_enable */
int cache_sim_reuse_enable(cache_handle_t *cache) {
    /* sanity check */
    if (NULL != cache->reuse) {
        return CACHE_SIM_ERROR;
    }

    /* variables declaration */
    list_t *list = NULL;

    /* allocate memory for to calculate cache reuse distance */
    cache->reuse = malloc(sizeof(list_t));
    if (NULL == cache->reuse) {
        printf("unable to allocate memory to calculate reuse distance\n");
        return CACHE_SIM_ERROR;
    }
    list = cache->reuse;

    /* initialize the list */
    list->head.next = &(list->head);
    list->head.prev = &(list->head);
    list->len = 0;

    printf("--------------------------------\n");
    printf("Reuse distance calculation is ON\n");
    printf("Memory required: %d bytes +%d/l\n", sizeof(list_t), sizeof(list_item_t));
    printf("--------------------------------\n");

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_reuse_disable */
int cache_sim_reuse_disable(cache_handle_t *cache) {
    if (NULL != cache->reuse) {
        free(cache->reuse);

        // TODO: print something nice here...
        printf("--------------------------------\n");
        printf("Reuse distance calculation is OFF\n");
        printf("--------------------------------\n");

        return CACHE_SIM_SUCCESS;
    }
    return CACHE_SIM_ERROR;
}

/* cache_sim_reuse */
int cache_sim_reuse(cache_handle_t *cache, const uint64_t line_id) {
    /* variable declarations */
    list_item_t *item;

    /* for all elements in the list of lines... */
    for (item = (list_item_t *)((list_t *)cache->reuse)->head.next;
        item != &(((list_t *)cache->reuse)->head);
        item = (list_item_t *)item->next) {
        /* ...if we find the line... */
        if (line_id == item->line_id) {
            /* ...report it... */
            #ifdef DEBUG
            printf("REUSE  line id [%d] reuse distance [%d]\n",
                item->line_id, item->age);
            #endif

            /* ...move the line to the top of the list... */
            list_movetop_item((list_t *)cache->reuse, item);

            /* ...set the line's age to zero */
            item->age = 0;

            return CACHE_SIM_SUCCESS;
        }

        /* increment age for every line above the line_id we are looking for */
        item->age++;
    }

    /* if the line was not found, add it to the list */
    item = malloc(sizeof(list_item_t));
    if (NULL == item) {
        return CACHE_SIM_ERROR;
    }
    list_prepend_item((list_t *)cache->reuse, item);
    item->line_id = line_id;
    item->age = 0;

    #ifdef DEBUG
    printf("USE    line id [%d]\n", item->line_id);
    #endif

    return CACHE_SIM_SUCCESS;
}

// EOF
