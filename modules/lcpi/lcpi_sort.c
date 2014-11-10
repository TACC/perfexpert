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
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Modules headers */
#include "lcpi.h"
#include "lcpi_types.h"
#include "lcpi_sort.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* Sorting orders */
static sort_t orders[] = {
    { "relevance",   &cmp_relevance },
    { "performance", &cmp_performance },
    { "mixed",       &cmp_mixed }, // Sorting mixed? It makes no sense :/
    { NULL,          NULL}
};

/* hotspot_sort */
int hotspot_sort(perfexpert_list_t *profiles) {
    perfexpert_list_item_t **items, *item;
    int i = 0, n = 0, index = 0;
    lcpi_hotspot_t *h = NULL;
    lcpi_profile_t *p;

    OUTPUT_VERBOSE((2, "%s", _BLUE("Sorting hotspots")));

    /* Find the sorting function for the requested order */
    while (NULL != orders[i].name) {
        if (0 == strcmp(my_module_globals.order, orders[i].name)) {
            /* For each profile in the list of profiles... */
            perfexpert_list_for(p, profiles, lcpi_profile_t) {
                OUTPUT_VERBOSE((7, "   sorting %s by %s", _YELLOW(p->name),
                    my_module_globals.order));

                OUTPUT_VERBOSE((10, "   %s", _CYAN("original order")));
                perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
                    OUTPUT_VERBOSE((10, "      [%f] [%f] %s", h->importance,
                        lcpi_get_value(h->metrics_by_name, "overall"),
                        h->name));
                }

                if (0 == perfexpert_list_get_size(&(p->hotspots))) {
                    return PERFEXPERT_SUCCESS;
                }
                items = (perfexpert_list_item_t**)malloc(
                    sizeof(perfexpert_list_item_t*) *
                    perfexpert_list_get_size(&(p->hotspots)));

                if (NULL == items) {
                    return PERFEXPERT_ERROR;
                }

                while (NULL != (item =
                    perfexpert_list_get_first(&(p->hotspots)))) {
                    perfexpert_list_remove_item(&(p->hotspots), item);
                    items[index++] = item;
                }

                qsort(items, index, sizeof(perfexpert_list_item_t*),
                    (int(*)(const void*, const void*))(*orders[i].function));

                for (n = 0; n < index; n++) {
                    perfexpert_list_append(&(p->hotspots), items[n]);
                }
                free(items);

                OUTPUT_VERBOSE((10, "   %s", _CYAN("sorted order")));
                perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
                    OUTPUT_VERBOSE((10, "      [%f] [%f] %s", h->importance,
                        lcpi_get_value(h->metrics_by_name, "overall"),
                        h->name));
                }
            }
            return PERFEXPERT_SUCCESS;
        }
        i++;
    }
    OUTPUT(("%s unknown sorting order (%s), hotspots will not be sorted",
        _BOLDRED("WARNING:"), my_module_globals.order));

    return PERFEXPERT_SUCCESS;
}

static int cmp_relevance(const lcpi_hotspot_t **a, const lcpi_hotspot_t **b) {
    if ((*a)->importance > (*b)->importance) return 1;
    if ((*a)->importance < (*b)->importance) return -1;
    if ((*a)->importance == (*b)->importance) return 0;
}

static int cmp_performance(const lcpi_hotspot_t **a, const lcpi_hotspot_t **b) {
    if (isnan(lcpi_get_value((*a)->metrics_by_name, "overall"))) return 1;
    if (isnan(lcpi_get_value((*b)->metrics_by_name, "overall"))) return -1;
    if (isinf(lcpi_get_value((*a)->metrics_by_name, "overall"))) return 1;
    if (isinf(lcpi_get_value((*b)->metrics_by_name, "overall"))) return -1;
    if (lcpi_get_value((*a)->metrics_by_name, "overall") >
        lcpi_get_value((*b)->metrics_by_name, "overall")) return -1;
    if (lcpi_get_value((*a)->metrics_by_name, "overall") <
        lcpi_get_value((*b)->metrics_by_name, "overall")) return 1;
    if (lcpi_get_value((*a)->metrics_by_name, "overall") ==
        lcpi_get_value((*b)->metrics_by_name, "overall")) return 0;
}

static int cmp_mixed(const lcpi_hotspot_t **a, const lcpi_hotspot_t **b) {
    if (isnan(lcpi_get_value((*a)->metrics_by_name, "overall"))) return 1;
    if (isnan(lcpi_get_value((*b)->metrics_by_name, "overall"))) return -1;
    if (isinf(lcpi_get_value((*a)->metrics_by_name, "overall"))) return 1;
    if (isinf(lcpi_get_value((*b)->metrics_by_name, "overall"))) return -1;
    if ((lcpi_get_value((*a)->metrics_by_name, "overall") * (*a)->importance) >
        (lcpi_get_value((*b)->metrics_by_name, "overall") * (*b)->importance))
        return -1;
    if ((lcpi_get_value((*a)->metrics_by_name, "overall") * (*a)->importance) <
        (lcpi_get_value((*b)->metrics_by_name, "overall") * (*b)->importance))
        return 1;
    if ((lcpi_get_value((*a)->metrics_by_name, "overall") * (*a)->importance) ==
        (lcpi_get_value((*b)->metrics_by_name, "overall") * (*b)->importance))
        return 0;
}

#ifdef __cplusplus
}
#endif

// EOF
