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

/* Modules headers */
#include "lcpi.h"
#include "lcpi_types.h"
#include "lcpi_sort.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* Sorting orders */
static sort_t orders[] = {
    {"relevance",   &sort_by_relevance},
    {"performance", &sort_by_performance},
    {"mixed",       &sort_by_mixed}, // Sorting mixed? It makes no sense :/
    {NULL, NULL}
};

/* hotspot_sort */
int hotspot_sort(perfexpert_list_t *profiles) {
    lcpi_profile_t *p;
    int i = 0;

    OUTPUT_VERBOSE((2, "%s", _BLUE("Sorting hotspots")));

    /* Find the sorting function for the requested order */
    while (NULL != orders[i].name) {
        if (0 == strcmp(my_module_globals.order, orders[i].name)) {
            /* For each profile in the list of profiles... */
            perfexpert_list_for(p, profiles, lcpi_profile_t) {
                OUTPUT_VERBOSE((7, "   sorting %s by %s", _YELLOW(p->name),
                    my_module_globals.order));

                /* Call the sorting function */
                if (PERFEXPERT_SUCCESS != (*orders[i].function)(p)) {
                    OUTPUT(("%s", _ERROR("Error: unable to sort hotspots")));
                    return PERFEXPERT_ERROR;
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

/* sort_by_relevance */
static int sort_by_relevance(lcpi_profile_t *profile) {
    lcpi_hotspot_t *i = NULL, *j = NULL;
    int swapped = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((10, "   %s", _CYAN("original order")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        swapped = PERFEXPERT_FALSE;
        perfexpert_list_reverse_for(j, &(profile->hotspots), lcpi_hotspot_t) {
            if (j->importance < ((lcpi_hotspot_t *)j->prev)->importance) {
                perfexpert_list_swap((perfexpert_list_item_t *)j,
                    (perfexpert_list_item_t *)j->prev);
                swapped = PERFEXPERT_TRUE;
            }
        }
        if (PERFEXPERT_FALSE == swapped) {
            break;
        }
    }

    OUTPUT_VERBOSE((10, "   %s", _CYAN("sorted order (relevance)")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    OUTPUT_VERBOSE((10, "   %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

/* sort_by_performance */
static int sort_by_performance(lcpi_profile_t *profile) {
    lcpi_hotspot_t *i = NULL, *j = NULL;
    int swapped = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((10, "   %s", _CYAN("original order")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        swapped = PERFEXPERT_FALSE;
        perfexpert_list_reverse_for(j, &(profile->hotspots), lcpi_hotspot_t) {
            if (lcpi_get_value(j->metrics_by_name, "overall") < lcpi_get_value(
                ((lcpi_hotspot_t *)j->prev)->metrics_by_name, "overall")) {
                perfexpert_list_swap((perfexpert_list_item_t *)j,
                    (perfexpert_list_item_t *)j->prev);
                swapped = PERFEXPERT_TRUE;
            }
        }
        if (PERFEXPERT_FALSE == swapped) {
            break;
        }
    }

    OUTPUT_VERBOSE((10, "   %s", _CYAN("sorted order (relevance)")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    OUTPUT_VERBOSE((10, "   %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

/* sort_by_mixed */
static int sort_by_mixed(lcpi_profile_t *profile) {
    lcpi_hotspot_t *i = NULL, *j = NULL;
    int swapped = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((10, "   %s", _CYAN("original order")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        swapped = PERFEXPERT_FALSE;
        perfexpert_list_reverse_for(j, &(profile->hotspots), lcpi_hotspot_t) {
            if ((lcpi_get_value(j->metrics_by_name, "overall") * j->importance) <
                (lcpi_get_value(((lcpi_hotspot_t *)j->prev)->metrics_by_name,
                    "overall") * ((lcpi_hotspot_t *)j->prev)->importance)) {
                perfexpert_list_swap((perfexpert_list_item_t *)j,
                    (perfexpert_list_item_t *)j->prev);
                swapped = PERFEXPERT_TRUE;
            }
        }
        if (PERFEXPERT_FALSE == swapped) {
            break;
        }
    }

    OUTPUT_VERBOSE((10, "   %s", _CYAN("sorted order (relevance)")));
    perfexpert_list_for(i, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "      [%f] %s", i->importance, i->name));
    }

    OUTPUT_VERBOSE((10, "   %s", _MAGENTA("done")));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
