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

/* profile_aggregate_hotspots */
int profile_aggregate_hotspots(profile_t *profile) {
    procedure_t *hotspot;
    procedure_t *aggregated_hotspot;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("   Aggregating hotspots")));

    /* Filling the hotspot that should last */
    PERFEXPERT_ALLOC(procedure_t, aggregated_hotspot, sizeof(procedure_t));
    aggregated_hotspot->id = 0;
    aggregated_hotspot->line = 0;
    aggregated_hotspot->instructions = 0;
    aggregated_hotspot->name = profile->name;
    aggregated_hotspot->type = PERFEXPERT_HOTSPOT_PROGRAM;
    aggregated_hotspot->file = NULL;
    aggregated_hotspot->module = NULL;
    aggregated_hotspot->metrics_by_id = NULL;
    perfexpert_list_construct(&(aggregated_hotspot->metrics));
    perfexpert_list_item_construct(
        (perfexpert_list_item_t *)aggregated_hotspot);

    /* For each hotspot... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        metric_t *metric;

        OUTPUT_VERBOSE((10,
            "      moving %d metrics from %s to the aggregated hotspot",
            (int)perfexpert_list_get_size(&(hotspot->metrics)),
            _RED(hotspot->name)));

        /* For all metrics in hotspot... */
        while (0 != perfexpert_list_get_size(&(hotspot->metrics))) {
            metric = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));

            /* Move this metric to the aggregated hotspot's list of metrics */
            perfexpert_list_remove_item(&(hotspot->metrics),
                (perfexpert_list_item_t *)metric);
            perfexpert_list_append(
                (perfexpert_list_t *)&(aggregated_hotspot->metrics),
                (perfexpert_list_item_t *)metric);
        }

        /* Remove hotspot from the profile's list of hotspots and move on */
        perfexpert_list_remove_item(&(profile->hotspots), 
            (perfexpert_list_item_t *)hotspot);
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    /* Add the aggregated hotspot to the profile's list of hotspots */
    perfexpert_list_append(&(profile->hotspots), 
        (perfexpert_list_item_t *)aggregated_hotspot);

    OUTPUT_VERBOSE((10, "      aggregated hotspot has %d metrics",
        perfexpert_list_get_size(&(aggregated_hotspot->metrics))));

    return PERFEXPERT_SUCCESS;
}

/* profile_aggregate_metrics */
int profile_aggregate_metrics(procedure_t *hotspot) {
    metric_t *current = NULL;
    metric_t *next = NULL;
    metric_t *next_next = NULL;

    if (-1 == globals.thread) {
        OUTPUT_VERBOSE((5, "      %s (metrics count: %d)",
            _YELLOW("Aggregate metrics ignoring thread ID"),
            perfexpert_list_get_size(&(hotspot->metrics))));
    } else {
        OUTPUT_VERBOSE((5, "      %s (%d) (metrics count: %d)",
            _YELLOW("Aggregate metrics by thread ID"), globals.thread,
            perfexpert_list_get_size(&(hotspot->metrics))));
    }

    /* Ignore and remove from list other thread IDs */
    current = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
    while ((perfexpert_list_item_t *)current != &(hotspot->metrics.sentinel)) {
        next = (metric_t *)current->next;

        if ((-1 != globals.thread) && (current->thread != globals.thread)) {
            perfexpert_list_remove_item(&(hotspot->metrics),
                (perfexpert_list_item_t *)current);
            free(current);
        }
        if (-1 == globals.thread) {
            current->thread = 0;
        }
        current = next;
    }

    /* For all metrics in hotspot... */
    current = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
    while ((perfexpert_list_item_t *)current != &(hotspot->metrics.sentinel)) {
        if (9 <= globals.verbose_level) {
            printf("%s          %s(%d)[%d] ", PROGRAM_PREFIX,
                _GREEN(current->name), current->thread, current->experiment);
        }

        /* Compare current metric with the next metric in the list */
        next = (metric_t *)current->next;
        while ((perfexpert_list_item_t *)next != &(hotspot->metrics.sentinel)) {
            next_next = (metric_t *)next->next;

            if ((0 == strcmp(current->name, next->name)) &&
                (current->experiment == next->experiment)) {

                current->value += next->value;
                perfexpert_list_remove_item(&(hotspot->metrics),
                    (perfexpert_list_item_t *)next);
                free(next);

                if (10 <= globals.verbose_level) {
                    printf("%s ", _CYAN("removed"));
                }
            } else {
                if (10 <= globals.verbose_level) {
                    printf("%s(%d)[%d] ", next->name, next->thread,
                        next->experiment);
                }
            }
            /* Compare to the next metric */
            next = next_next;
        }
        if (9 <= globals.verbose_level) {
            printf("\n");
            fflush(stdout);
        }

        /* Do the same with the next metric */
        current = (metric_t *)perfexpert_list_get_next(current);
    }

    OUTPUT_VERBOSE((5, "         new metrics count: %d",
        perfexpert_list_get_size(&(hotspot->metrics))));

    if (10 <= globals.verbose_level) {
        printf("%s          ", PROGRAM_PREFIX);
        current = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
        while ((perfexpert_list_item_t *)current !=
            &(hotspot->metrics.sentinel)) {
            printf("%s[%d](%d) ", _MAGENTA(current->name), current->thread,
                current->experiment);
            current = (metric_t *)perfexpert_list_get_next(current);
        }
        printf("\n");
        fflush(stdout);
    }

    return PERFEXPERT_SUCCESS;
}

/* profile_flatten_all */
int profile_flatten_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Flattening profiles")));

    /* For each profile in the list of profiles... */
    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((10, " [%d] %s", profile->id, _GREEN(profile->name)));

        /* Aggregate all hotspots in this profile? */
        if (PERFEXPERT_TRUE == globals.aggregate) {
            if (PERFEXPERT_SUCCESS != profile_aggregate_hotspots(profile)) {
                OUTPUT(("%s (%s)",
                    _ERROR("Error: unable to aggregate profile's hotspots"),
                    profile->name));
                return PERFEXPERT_ERROR;
            }
            profile = (profile_t *)perfexpert_list_get_first(profiles);
        }

        /* Flatten all hotspots in this profile */
        if (PERFEXPERT_SUCCESS != profile_flatten_hotspots(profile)) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to flatten profile"),
                profile->name));
            return PERFEXPERT_ERROR;
        }

        /* Move on */
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }
    return PERFEXPERT_SUCCESS;
}

/* profile_flatten_hotspots */
int profile_flatten_hotspots(profile_t *profile) {
    procedure_t *hotspot = NULL;

    OUTPUT_VERBOSE((4, "   %s", _YELLOW("Flatenning hotspots")));

    /* For each hotspot in the profile's list of hotspots... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        OUTPUT_VERBOSE((8, "    [%d] %s (%s@%s:%d)", hotspot->id,
            _RED(hotspot->name),
            hotspot->module != NULL ? hotspot->module->shortname : "---",
            hotspot->file != NULL ? hotspot->file->shortname : "---",
            hotspot->line));

        /* Aggregate threads measurements */
        if (PERFEXPERT_SUCCESS != profile_aggregate_metrics(hotspot)) {
            OUTPUT(("%s (%s)", _ERROR("Error: aggregating metrics"),
                hotspot->name));
            return PERFEXPERT_ERROR;
        }

        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    /* Calculate importance and variance */
    if (PERFEXPERT_SUCCESS != calculate_importance_variance(profile)) {
        OUTPUT(("%s (%s)", _ERROR("Error: calculating importance and variance"),
            hotspot->name));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* profile_check_all */
int profile_check_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Checking profiles")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((10, " [%d] %s", profile->id, _GREEN(profile->name)));
        if (0 < perfexpert_list_get_size(&(profile->callees))) {
           if (PERFEXPERT_SUCCESS !=
            profile_check_callpath(&(profile->callees), 0)) {
                OUTPUT(("%s (%s)", _ERROR("Error: malformed profile"),
                    profile->name));
                return PERFEXPERT_ERROR;
           }
        }
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }
    return PERFEXPERT_SUCCESS;
}

/* profile_check_callpath */
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
        OUTPUT_VERBOSE((9, "%s [%d] %s (%s@%s:%d)", indent, callpath->id,
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
