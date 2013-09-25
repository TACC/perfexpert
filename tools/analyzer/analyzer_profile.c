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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <strings.h>

/* PerfExpert headers */
#include "analyzer.h"
#include "analyzer_profile.h"
#include "analyzer_hpctoolkit.h"
#include "analyzer_vtune.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Tools definition */
tool_t tools[] = {
    {
        "hpctoolkit",
        &hpctoolkit_parse_file,
        PERFEXPERT_TOOL_HPCTOOLKIT_TOT_INS,
        PERFEXPERT_TOOL_HPCTOOLKIT_TOT_CYC,
        PERFEXPERT_TOOL_HPCTOOLKIT_COUNTERS
    }, {
        "vtune",
        &vtune_parse_file,
        PERFEXPERT_TOOL_VTUNE_TOT_INS,
        PERFEXPERT_TOOL_VTUNE_TOT_CYC,
        PERFEXPERT_TOOL_VTUNE_COUNTERS
    }, {NULL, NULL, NULL, NULL}
};

/* profile_parse_file */
int profile_parse_file(const char* file, const char* tool,
    perfexpert_list_t *profiles) {
    int i = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Measurements phase")));

    /* Find the measurement function for this tool */
    while (NULL != tools[i].name) {
        if (0 == strcmp(tool, tools[i].name)) {
            OUTPUT_VERBOSE((2, "%s (%s)", "Parsing experiments", tool));
            /* Call the measurement function for this tool */
            return (*tools[i].function)(file, profiles);
        }
        i++;
    }

    OUTPUT(("%s [%s]", _ERROR("Error: unknown measurement tool"), tool));
    return PERFEXPERT_ERROR;
}

/* profile_aggregate_hotspots */
int profile_aggregate_hotspots(profile_t *profile) {
    procedure_t *hotspot = NULL;
    procedure_t *aggregated_hotspot = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("   Aggregating hotspots")));

    /* Filling the hotspot that should last */
    PERFEXPERT_ALLOC(procedure_t, aggregated_hotspot, sizeof(procedure_t));
    aggregated_hotspot->id = 0;
    aggregated_hotspot->line = 0;
    aggregated_hotspot->valid = PERFEXPERT_TRUE;
    aggregated_hotspot->cycles = 0.0;
    aggregated_hotspot->variance = 0.0;
    aggregated_hotspot->importance = 0.0;
    aggregated_hotspot->instructions = 0.0;
    aggregated_hotspot->name = profile->name;
    aggregated_hotspot->type = PERFEXPERT_HOTSPOT_PROGRAM;
    aggregated_hotspot->file = NULL;
    aggregated_hotspot->module = NULL;
    aggregated_hotspot->lcpi_by_name = NULL;
    aggregated_hotspot->metrics_by_id = NULL;
    aggregated_hotspot->metrics_by_name = NULL;
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
int profile_aggregate_metrics(profile_t *profile, procedure_t *hotspot) {
    metric_t *current = NULL;
    metric_t *next = NULL;
    metric_t *next_next = NULL;

    if (-1 == globals.thread) {
        OUTPUT_VERBOSE((5, "      %s (metrics count: %d)",
            _CYAN("Aggregate metrics ignoring thread ID"),
            perfexpert_list_get_size(&(hotspot->metrics))));
    } else {
        OUTPUT_VERBOSE((5, "      %s (%d) (metrics count: %d)",
            _CYAN("Aggregate metrics by thread ID"), globals.thread,
            perfexpert_list_get_size(&(hotspot->metrics))));
    }

    /* Ignore and remove from list of metrics the ones with other thread IDs */
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
        if (9 <= globals.verbose) {
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
                PERFEXPERT_DEALLOC(next);

                if (10 <= globals.verbose) {
                    printf("%s ", _CYAN("removed"));
                }
            } else {
                if (10 <= globals.verbose) {
                    printf("%s(%d)[%d] ", next->name, next->thread,
                        next->experiment);
                }
            }
            /* Compare to the next metric */
            next = next_next;
        }
        if (9 <= globals.verbose) {
            printf("\n");
            fflush(stdout);
        }

        /* Do the same with the next metric */
        current = (metric_t *)perfexpert_list_get_next(current);
    }

    OUTPUT_VERBOSE((5, "      %s (%d)", _MAGENTA("new metrics count"),
        perfexpert_list_get_size(&(hotspot->metrics))));

    /* Hash metric by name (at this point it should be unique) */
    if (10 <= globals.verbose) {
        printf("%s          ", PROGRAM_PREFIX);
    }
    current = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
    while ((perfexpert_list_item_t *)current != &(hotspot->metrics.sentinel)) {
        if (10 <= globals.verbose) {
            printf("%s[%d](%d) ", _CYAN(current->name), current->thread,
                current->experiment);
        }
        strcpy(current->name_md5, perfexpert_md5_string(current->name));
        perfexpert_hash_add_str(hotspot->metrics_by_name, name_md5, current);
        current = (metric_t *)perfexpert_list_get_next(current);
    }
    if (10 <= globals.verbose) {
        printf("\n");
        fflush(stdout);
    }

    return PERFEXPERT_SUCCESS;
}

/* profile_flatten_all */
int profile_flatten_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Flattening profiles")));

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
    procedure_t *hotspot_prev = NULL;
    procedure_t *hotspot = NULL;
    metric_t *metric = NULL;
    char key_md5[33];

    OUTPUT_VERBOSE((4, "   %s", _CYAN("Flatenning hotspots")));

    /* For each hotspot in the profile's list of hotspots... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        if (PERFEXPERT_HOTSPOT_FUNCTION == hotspot->type) {
            OUTPUT_VERBOSE((8, "    [%d] %s (%s@%s:%d)", hotspot->id,
                _YELLOW(hotspot->name),
                hotspot->module != NULL ? hotspot->module->shortname : "---",
                hotspot->file != NULL ? hotspot->file->shortname : "---",
                hotspot->line));
        }
        if (PERFEXPERT_HOTSPOT_LOOP == hotspot->type) {
            loop_t *loop = (loop_t *)hotspot;
            OUTPUT_VERBOSE((8, "    [%d] %s (%s@%s:%d)", hotspot->id,
                _YELLOW("loop"), loop->procedure->module->shortname,
                loop->procedure->file->shortname, hotspot->line));
        }

        /* Aggregate threads measurements */
        if (PERFEXPERT_SUCCESS != profile_aggregate_metrics(profile, hotspot)) {
            OUTPUT(("%s (%s)", _ERROR("Error: aggregating metrics"),
                hotspot->name));
            return PERFEXPERT_ERROR;
        }

        /* Check if the total number of instructions is present, if not delete
         * this hotspot from the list and move on, but first save the total
         * cycles (if present)!
         */
        strcpy(key_md5,
            perfexpert_md5_string(perfexpert_tool_get_tot_ins(globals.tool)));
        perfexpert_hash_find_str(hotspot->metrics_by_name, key_md5, metric);
        if (NULL == metric) {
            strcpy(key_md5, perfexpert_md5_string(perfexpert_tool_get_tot_cyc(
                globals.tool)));
            perfexpert_hash_find_str(hotspot->metrics_by_name, key_md5, metric);
            if (NULL != metric) {
                profile->cycles += metric->value;
            }

            hotspot_prev = (procedure_t *)hotspot->prev;
            perfexpert_list_remove_item(&(profile->hotspots),
                (perfexpert_list_item_t *)hotspot);
            hotspot = hotspot_prev;

            OUTPUT_VERBOSE((8, "      %s (%s not found)",
                _RED("removed from list of hotspots"),
                perfexpert_tool_get_tot_cyc(globals.tool)));
        }
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    /* Calculate importance and variance */
    if (PERFEXPERT_SUCCESS != calculate_importance_variance(profile)) {
        OUTPUT(("%s (%s)", _ERROR("Error: calculating importance and variance"),
            hotspot->name));
        return PERFEXPERT_ERROR;
    }

    /* Compute LCPI */
    if (PERFEXPERT_SUCCESS != lcpi_compute(profile)) {
        OUTPUT(("%s (%s)", _ERROR("Error: calculating LCPI"), hotspot->name));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* profile_check_all */
int profile_check_all(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Checking profiles")));

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
    int i = 0;

    PERFEXPERT_ALLOC(char, indent, ((2 * root) + 1));
    for (i = 0; i <= root; i++) {
        strcat(indent, " .");
    }

    callpath = (callpath_t *)perfexpert_list_get_first(calls);
    while ((perfexpert_list_item_t *)callpath != &(calls->sentinel)) {
        if (PERFEXPERT_HOTSPOT_FUNCTION == callpath->procedure->type) {
            OUTPUT_VERBOSE((9, "%s [%d] %s (%s@%s:%d)", indent, callpath->id,
                _YELLOW(callpath->procedure->name),
                callpath->procedure->module->shortname,
                callpath->procedure->file->shortname,
                callpath->procedure->line));
        }
        if (PERFEXPERT_HOTSPOT_LOOP == callpath->procedure->type) {
            loop_t *loop = (loop_t *)callpath->procedure;
            OUTPUT_VERBOSE((9, "%s [%d] %s (%s@%s:%d)", indent, callpath->id,
                _YELLOW("loop"), loop->procedure->module->shortname,
                loop->procedure->file->shortname, loop->line));
        }

        if (0 < perfexpert_list_get_size(&(callpath->callees))) {
            root++;
            profile_check_callpath(&(callpath->callees), root);
            root--;
        }
        callpath = (callpath_t *)perfexpert_list_get_next(callpath);
    }
    PERFEXPERT_DEALLOC(indent);
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
