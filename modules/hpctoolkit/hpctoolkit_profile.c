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
#include <strings.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_types.h"
#include "hpctoolkit_profile.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* profile_aggregate_metrics */
static int profile_aggregate_metrics(hpctoolkit_procedure_t *hotspot) {
    hpctoolkit_metric_t *c = NULL, *n = NULL, *p = NULL;

    perfexpert_list_for(c, &(hotspot->metrics), hpctoolkit_metric_t) {
        perfexpert_list_for(n, &(hotspot->metrics), hpctoolkit_metric_t) {
            if ((c != n) && (0 == strcmp(c->name, n->name)) &&
                (c->experiment == n->experiment) && (c->thread == n->thread) &&
                (c->mpi_rank == n->mpi_rank)) {

                c->value += n->value;
                p = (hpctoolkit_metric_t *)n->prev;
                perfexpert_list_remove_item(&(hotspot->metrics),
                    (perfexpert_list_item_t *)n);
                PERFEXPERT_DEALLOC(n);
                n = p;
            }
        }
        OUTPUT_VERBOSE((9, "         %s (%g) [tid=%d] [rank=%d] [exp=%d]",
            _GREEN(c->name), c->value, c->thread, c->mpi_rank, c->experiment));
    }
    OUTPUT_VERBOSE((5, "      %s (%d)", _MAGENTA("metric count"),
        perfexpert_list_get_size(&(hotspot->metrics))));

    return PERFEXPERT_SUCCESS;
}

/* profile_flatten_all */
int profile_flatten_all(perfexpert_list_t *profiles) {
    hpctoolkit_profile_t *p = NULL;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Flattening profiles")));

    perfexpert_list_for(p, profiles, hpctoolkit_profile_t) {
        OUTPUT_VERBOSE((10, " [%d] %s", p->id, _GREEN(p->name)));
        if (PERFEXPERT_SUCCESS != profile_flatten_hotspots(p)) {
            OUTPUT(("%s (%s)", _ERROR("flattening profile"), p->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* profile_flatten_hotspots */
static int profile_flatten_hotspots(hpctoolkit_profile_t *profile) {
    hpctoolkit_procedure_t *h = NULL;

    OUTPUT_VERBOSE((4, "   %s", _CYAN("Flatenning hotspots")));

    perfexpert_list_for(h, &(profile->hotspots), hpctoolkit_procedure_t) {
        if (PERFEXPERT_HOTSPOT_FUNCTION == h->type) {
            OUTPUT_VERBOSE((8, "    [%d] %s (%s@%s:%d)", h->id,
                _YELLOW(h->name),
                h->module != NULL ? h->module->shortname : "---",
                h->file != NULL ? h->file->shortname : "---", h->line));
        }
        if (PERFEXPERT_HOTSPOT_LOOP == h->type) {
            hpctoolkit_loop_t *l = (hpctoolkit_loop_t *)h;
            OUTPUT_VERBOSE((8, "    [%d] %s (%s@%s:%d)", h->id, _YELLOW("loop"),
                l->procedure->module->shortname, l->procedure->file->shortname,
                h->line));
        }

        if (PERFEXPERT_SUCCESS != profile_aggregate_metrics(h)) {
            OUTPUT(("%s (%s)", _ERROR("aggregating metrics"), h->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* profile_check_all */
int profile_check_all(perfexpert_list_t *profiles) {
    hpctoolkit_profile_t *p = NULL;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Checking profiles")));

    perfexpert_list_for(p, profiles, hpctoolkit_profile_t) {
        OUTPUT_VERBOSE((10, " [%d] %s", p->id, _GREEN(p->name)));
        if (0 < perfexpert_list_get_size(&(p->callees))) {
            if (PERFEXPERT_SUCCESS !=
                profile_check_callpath(&(p->callees), 0)) {
                OUTPUT(("%s (%s)", _ERROR("bad profile"), p->name));
                return PERFEXPERT_ERROR;
           }
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* profile_check_callpath */
static int profile_check_callpath(perfexpert_list_t *calls, int root) {
    hpctoolkit_callpath_t *c = NULL;
    char indent[256];
    int i = 0;

    bzero(indent, 256);
    if (127 < root) {
        root = 127;
    }
    for (i = 0; i <= root; i++) {
        strcat(indent, " .");
    }

    perfexpert_list_for(c, calls, hpctoolkit_callpath_t) {
        if (PERFEXPERT_HOTSPOT_FUNCTION == c->procedure->type) {
            OUTPUT_VERBOSE((9, "%s [%d] %s (%s@%s:%d)", indent, c->id,
                _YELLOW(c->procedure->name), c->procedure->module->shortname,
                c->procedure->file->shortname, c->procedure->line));
        }
        if (PERFEXPERT_HOTSPOT_LOOP == c->procedure->type) {
            hpctoolkit_loop_t *l = (hpctoolkit_loop_t *)c->procedure;
            OUTPUT_VERBOSE((9, "%s [%d] %s (%s@%s:%d)", indent, c->id,
                _YELLOW("loop"), l->procedure->module->shortname,
                l->procedure->file->shortname, l->line));
        }
        if (0 < perfexpert_list_get_size(&(c->callees))) {
            root++;
            profile_check_callpath(&(c->callees), root);
            root--;
        }
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
