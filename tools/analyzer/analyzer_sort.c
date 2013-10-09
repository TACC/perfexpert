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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <float.h>

/* PerfExpert headers */
#include "analyzer.h" 
#include "analyzer_sort.h" 
#include "perfexpert_constants.h"
#include "perfexpert_list.h"

/* Sorting orders */
static sort_t orders[] = {
    {"relevance",   &sort_by_relevance},
    {"performance", &sort_by_performance},
    {"mixed",       &sort_by_mixed}, // Sorting mixed? It makes no sense :/
    {NULL, NULL}
};

/* hotspot_sort */
int hotspot_sort(perfexpert_list_t *profiles) {
    profile_t *profile;
    int i = 0;

    OUTPUT_VERBOSE((2, "%s", _BLUE("Sorting hotspots")));

    /* Find the sorting function for the requested order */
    while (NULL != orders[i].name) {
        if (0 == strcmp(globals.order, orders[i].name)) {
            /* For each profile in the list of profiles... */
            profile = (profile_t *)perfexpert_list_get_first(profiles);
            while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
                OUTPUT_VERBOSE((7, "   sorting [%d] %s by %s", profile->id,
                    _YELLOW(profile->name), globals.order));

                /* Call the sorting function */
                if (PERFEXPERT_SUCCESS != (*orders[i].function)(profile)) {
                    OUTPUT(("%s", _ERROR("Error: unable to sort hotspots")));
                    return PERFEXPERT_ERROR;
                }

                /* Move on */
                profile = (profile_t *)perfexpert_list_get_next(profile);
            }
            return PERFEXPERT_SUCCESS;
        }
        i++;
    }
    OUTPUT(("   %s unknown sorting order (%s), hotspots are not sorted",
        _RED("WARNING:"), globals.order));

    return PERFEXPERT_ERROR;
}

/* sort_by_relevance */
static int sort_by_relevance(profile_t *profile) {
    procedure_t *h = NULL, *h2 = NULL, *h_max = NULL;
    perfexpert_list_t sorted;
    double relevance_max = -1;
    int i = 0;

    OUTPUT_VERBOSE((10, "      %s", _CYAN("original order")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s", h->importance, h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    perfexpert_list_construct(&sorted);

    for (h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        0 < perfexpert_list_get_size(&(profile->hotspots));
        h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots))) {

        relevance_max = -1;
        h_max = h;

        /* Find the highest importance */
        h2 = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        h2 = (procedure_t *)h2->next;
        for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)) - 1;
            h2 = (procedure_t *)perfexpert_list_get_next(h2), i++) {
            if (relevance_max <= h2->importance) {
                relevance_max = h2->importance;
                h_max = h2;
            }
        }
        /* Remove h_max from unsorted list */
        perfexpert_list_remove_item(&(profile->hotspots),
            (perfexpert_list_item_t *)h_max);

        /* Append h_max to sorted list */
        perfexpert_list_append(&sorted, (perfexpert_list_item_t *)h_max);
    }

    /* Copy list to unsorted_list and destroy profile's list of hotspot */
    profile->hotspots.sentinel.next = sorted.sentinel.next;
    profile->hotspots.sentinel.prev = sorted.sentinel.prev;
    profile->hotspots.length = sorted.length;
    profile->hotspots.sentinel.next->prev = &(profile->hotspots.sentinel);
    profile->hotspots.sentinel.prev->next = &(profile->hotspots.sentinel);

    OUTPUT_VERBOSE((10, "      %s", _CYAN("sorted order (relevance)")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s", h->importance, h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    OUTPUT_VERBOSE((10, "      %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

/* sort_by_performance */
static int sort_by_performance(profile_t *profile) {
    procedure_t *h = NULL, *h2 = NULL, *h_max = NULL;
    perfexpert_list_t sorted;
    double worst_overall = -1;
    int i = 0;

    OUTPUT_VERBOSE((10, "      %s", _CYAN("original order")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s",
            perfexpert_lcpi_hotspot_get(h, "overall"), h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    perfexpert_list_construct(&sorted);

    for (h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        0 < perfexpert_list_get_size(&(profile->hotspots));
        h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots))) {

        worst_overall = -1;
        h_max = h;

        /* Find the highest importance */
        h2 = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        h2 = (procedure_t *)h2->next;
        for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)) - 1;
            h2 = (procedure_t *)perfexpert_list_get_next(h2), i++) {
            if (worst_overall <= perfexpert_lcpi_hotspot_get(h2, "overall")) {
                worst_overall = perfexpert_lcpi_hotspot_get(h2, "overall");
                h_max = h2;
            }
        }
        /* Remove h_max from unsorted list */
        perfexpert_list_remove_item(&(profile->hotspots),
            (perfexpert_list_item_t *)h_max);

        /* Append h_max to sorted list */
        perfexpert_list_append(&sorted, (perfexpert_list_item_t *)h_max);
    }

    /* Copy list to unsorted_list and destroy profile's list of hotspot */
    profile->hotspots.sentinel.next = sorted.sentinel.next;
    profile->hotspots.sentinel.prev = sorted.sentinel.prev;
    profile->hotspots.length = sorted.length;
    profile->hotspots.sentinel.next->prev = &(profile->hotspots.sentinel);
    profile->hotspots.sentinel.prev->next = &(profile->hotspots.sentinel);

    OUTPUT_VERBOSE((10, "      %s", _CYAN("sorted order (performance)")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s",
            perfexpert_lcpi_hotspot_get(h, "overall"), h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    OUTPUT_VERBOSE((10, "      %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

/* sort_by_mixed */
static int sort_by_mixed(profile_t *profile) {
    procedure_t *h = NULL, *h2 = NULL, *h_max = NULL;
    perfexpert_list_t sorted;
    double index = -1;
    int i = 0;

    OUTPUT_VERBOSE((10, "      %s", _CYAN("original order")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s",
            (perfexpert_lcpi_hotspot_get(h, "overall") * h->importance),
            h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    perfexpert_list_construct(&sorted);

    for (h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        0 < perfexpert_list_get_size(&(profile->hotspots));
        h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots))) {

        index = -1;
        h_max = h;

        /* Find the highest importance */
        h2 = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
        h2 = (procedure_t *)h2->next;
        for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)) - 1;
            h2 = (procedure_t *)perfexpert_list_get_next(h2), i++) {
            if (index <=
                (perfexpert_lcpi_hotspot_get(h2, "overall") * h2->importance)) {
                index = perfexpert_lcpi_hotspot_get(h2, "overall") *
                    h2->importance;
                h_max = h2;
            }
        }
        /* Remove h_max from unsorted list */
        perfexpert_list_remove_item(&(profile->hotspots),
            (perfexpert_list_item_t *)h_max);

        /* Append h_max to sorted list */
        perfexpert_list_append(&sorted, (perfexpert_list_item_t *)h_max);
    }

    /* Copy list to unsorted_list and destroy profile's list of hotspot */
    profile->hotspots.sentinel.next = sorted.sentinel.next;
    profile->hotspots.sentinel.prev = sorted.sentinel.prev;
    profile->hotspots.length = sorted.length;
    profile->hotspots.sentinel.next->prev = &(profile->hotspots.sentinel);
    profile->hotspots.sentinel.prev->next = &(profile->hotspots.sentinel);

    OUTPUT_VERBOSE((10, "      %s",
        _CYAN("sorted order (performance * relevance)")));
    h = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    for (i = 0; i < perfexpert_list_get_size(&(profile->hotspots)); i++) {
        OUTPUT_VERBOSE((10, "         [%f] %s",
            (perfexpert_lcpi_hotspot_get(h, "overall") * h->importance),
            h->name));
        h = (procedure_t *)perfexpert_list_get_next(h);
    }

    OUTPUT_VERBOSE((10, "      %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
