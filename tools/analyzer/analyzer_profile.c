/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane and Leonardo Fialho
 */

/* System standard headers */
#include <strings.h>

/* PerfExpert headers */
#include "analyzer.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

int profile_flatten_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Flattening profiles")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((10, " [%d] %s", profile->id, _GREEN(profile->name)));
        if (PERFEXPERT_SUCCESS != profile_flatten(profile)) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to flatten profile"),
                profile->name));
            return PERFEXPERT_ERROR;
        }
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    return PERFEXPERT_SUCCESS;
}

int profile_flatten(profile_t *profile) {
    procedure_t *procedure;

    // for (procedure = profile->procedures_by_name; procedure != NULL;
    //     procedure = procedure->hh_int.next) {
    //     OUTPUT_VERBOSE((10, "  %s [%d] %s=%g", indent, metric->id,
    //         _CYAN(metric->name), metric->value));
    //     }

        // for (metric = callpath->procedure->metrics_by_id; metric != NULL;
        //     metric = metric->hh_int.next) {
        //     OUTPUT_VERBOSE((10, " .%s [%d] %s=%g [rank=%d] [tid=%d] [exp=%d]",
        //         indent, metric->id, _CYAN(metric->name), metric->value,
        //         metric->mpi_rank, metric->thread, metric->experiment));
        // }


    return PERFEXPERT_SUCCESS;
}

int profile_check_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Checking profiles")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((10, " [%d] %s", profile->id, _GREEN(profile->name)));
        if (0 < perfexpert_list_get_size(&(profile->callees))) {
           if (PERFEXPERT_SUCCESS != profile_check_callpath(&(profile->callees),
            0)) {
                OUTPUT(("%s (%s)", _ERROR("Error: malformed profile"),
                    profile->name));
                return PERFEXPERT_ERROR;
           }
        }
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    return PERFEXPERT_SUCCESS;
}

int profile_check_callpath(perfexpert_list_t *calls, int root) {
    callpath_t *callpath = NULL;
    char *indent = NULL;
    int i;

    PERFEXPERT_ALLOC(char, indent, ((2 * root) + 1));

    for (i = 0; i <= root; i++) {
        strcat(indent, " .");
    }

    callpath = (callpath_t *)perfexpert_list_get_first(calls);
    while ((perfexpert_list_item_t *)callpath != &(calls->sentinel)) {
        metric_t *metric = NULL;

        OUTPUT_VERBOSE((5, "%s [%d] %s (%s@%s:%d)", indent, callpath->id,
            _RED(callpath->procedure->name),
            callpath->procedure->module->shortname,
            callpath->procedure->file->shortname,
            callpath->procedure->line));

        if (0 < perfexpert_list_get_size(&(callpath->callees))) {
            root++;
            profile_check_callpath(&(callpath->callees), root);
            root--;
        }
        callpath = (callpath_t *)perfexpert_list_get_next(callpath);
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
