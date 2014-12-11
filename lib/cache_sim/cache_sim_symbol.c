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
#include <string.h>

/* Cache simulator headers */
#include "cache_sim.h"
#include "cache_sim_types.h"
#include "cache_sim_util.h"
#include "cache_sim_symbol.h"

/* cache_sim_symbol_enable */
int cache_sim_symbol_enable(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* sanity check: already enabled */
    if (NULL != cache->symbol_data) {
        printf("Warning: symbols tracking was already enabled\n");
        return CACHE_SIM_SUCCESS;
    }

    /* variables declaration */
    list_t *list = NULL;

    /* allocate memory for symbol tracking data area */
    cache->symbol_data = malloc(sizeof(list_t));
    if (NULL == cache->symbol_data) {
        printf("unable to allocate memory to track symbols\n");
        return CACHE_SIM_ERROR;
    }

    /* initialize the list */
    list = cache->symbol_data;
    list->head.next = &(list->head);
    list->head.prev = &(list->head);
    list->len = 0;

    printf("--------------------------------\n");
    printf("     Symbols tracking is ON     \n");
    printf("Memory required: %d bytes +%d/s\n", sizeof(list_t),
        sizeof(list_item_symbol_t));
    printf("--------------------------------\n");

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_symbol_disable */
int cache_sim_symbol_disable(cache_handle_t *cache) {
    /* sanity check: does cache exist? */
    if (NULL == cache) {
        printf("Error: cache does not exists\n");
        return CACHE_SIM_ERROR;
    }

    /* sanity check: already disabled */
    if (NULL == cache->symbol_data) {
        printf("Error: symbols tracking is not enabled\n");
        return CACHE_SIM_ERROR;
    }

    /* variables declaration */
    list_item_symbol_t *item = NULL;

    printf("--------------------------------\n");
    for (item = (list_item_symbol_t *)((list_t *)cache->symbol_data)->head.next;
        (void *)item != (void *)&(((list_t *)cache->symbol_data)->head);
        item = (list_item_symbol_t *)item->next) {
        printf("Symbol: %24s\n", item->symbol ? item->symbol : "(null)");
        printf("Cache accesses: %16"PRIu64"\n", item->access);
        printf("Cache hits:     %16"PRIu64"\n", item->hit);
        printf(" -> symbol hit rate  %10.2f%%\n",
            (((double)item->hit / (double)item->access) * 100));
        printf(" -> global hit rate  %10.2f%%\n",
            (((double)item->hit / (double)cache->hit) * 100));
        printf("Cache misses:   %16"PRIu64"\n", item->miss);
        printf(" -> symbol miss rate  %9.2f%%\n",
            (((double)item->miss / (double)item->access) * 100));
        printf(" -> global miss rate  %9.2f%%\n",
            (((double)item->miss / (double)cache->miss) * 100));
        printf("Set conflicts:  %16"PRIu64"\n", item->conflict);
        printf(" -> symbol conflict rate %6.2f%%\n",
            (((double)item->conflict / (double)item->miss) * 100));
        printf(" -> global conflict rate %6.2f%%\n",
            (((double)item->conflict / (double)cache->conflict) * 100));
        printf("--------------------------------\n");
    }
    printf("     Symbols tracking is OFF    \n");
    printf("--------------------------------\n");

    // TODO: free the list elements
    free(cache->symbol_data);

    return CACHE_SIM_SUCCESS;
}

/* cache_sim_symbol_access */
int cache_sim_symbol_access(cache_handle_t *cache, const uint64_t address,
    const char *symbol) {
    /* sanity check: is symbols tracking enabled? */
    if (NULL == cache->symbol_data) {
        if (CACHE_SIM_SUCCESS != cache_sim_symbol_enable(cache)) {
            printf("Error: unable to enable symbols tracking\n");
            return CACHE_SIM_ERROR;
        }
    }

    /* variable declarations */
    static int rc = CACHE_SIM_ERROR;
    list_item_symbol_t *item = NULL;

    /* for all elements in the list of symbols... */
    for (item = (list_item_symbol_t *)((list_t *)cache->symbol_data)->head.next;
        (void *)item != (void *)&(((list_t *)cache->symbol_data)->head);
        item = (list_item_symbol_t *)item->next) {
        /* ...if we find the symbol, stop */
        if (0 == strcmp(symbol, item->symbol)) {
            break;
        }
    }

    /* if symbol was not found allocate memory and add it to the list */
    if (&(((list_t *)(cache->symbol_data))->head) == (list_item_t *)item) {
        /* allocate memory for symbol item */
        item = malloc(sizeof(list_item_symbol_t));
        if (NULL == item) {
            printf("unable to allocate memory for symbol\n");
            return CACHE_SIM_ERROR;
        }

        /* initialize item */
        bzero(item->symbol, CACHE_SIM_SYMBOL_MAX_LENGTH);
        item->access   = 0;
        item->hit      = 0;
        item->miss     = 0;
        item->conflict = 0;

        /* copy the symbol name */
        if (CACHE_SIM_SYMBOL_MAX_LENGTH > (strlen(symbol) + 1)) {
            strcpy(item->symbol, symbol);
        } else {
            strncpy(item->symbol, symbol, (CACHE_SIM_SYMBOL_MAX_LENGTH - 1));
        }

        /* add the symbol to the list */
        list_prepend_item((list_t *)cache->symbol_data, (list_item_t *)item);

        #ifdef DEBUG
        printf("SYMBOL found [%s]\n", item->symbol);
        #endif
    }

    /* call the real access function and increment hit/miss counter */
    switch (rc = cache_sim_access(cache, address)) {
        /* hit on a prefetched line */
        case CACHE_SIM_L1_HIT + CACHE_SIM_L1_HIT_PREFETCH: // fallover
            /* increment prefetcher hits counter */
            item->prefetcher_hit++;
        /* hit on a pre-loaded cache line */
        case CACHE_SIM_L1_HIT:
            /* increment hits counter */
            item->hit++;
            break;

        /* miss on a prefetched line that have never been accessed */
        case CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT + CACHE_SIM_L1_MISS_CONFLICT: // fallover
            /* increment conflicts counter */
            item->conflict++;
        case CACHE_SIM_L1_MISS + CACHE_SIM_L1_PREFETCH_EVICT:
            /* increment prefetched evicted lines counter */
            item->prefetcher_evict++;
            /* increment misses counter */
            item->miss++;
            break;

        /* miss on a prefetched line that have never been accessed */
        case CACHE_SIM_L1_MISS + CACHE_SIM_L1_MISS_CONFLICT: // fallover
            /* increment conflicts counter */
            item->conflict++;
        /* common miss */
        case CACHE_SIM_L1_MISS:
            /* increment misses counter */
            item->miss++;
            break;
    }

    /* increment access counter */
    item->access++;

    return rc;
}

// EOF
