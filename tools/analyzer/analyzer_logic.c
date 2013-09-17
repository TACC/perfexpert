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
#include <float.h>
#include <stdio.h>

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"

/* calculate_importance_variance */
int calculate_importance_variance(profile_t *profile) {
    procedure_t *hotspot = NULL;
    metric_t *metric = NULL;
    double maximum = DBL_MIN, minimum = DBL_MAX;
    int i = 0;

    OUTPUT_VERBOSE((4, "   %s",
        _YELLOW("Calculating importance and variance")));

    /* Just in case... */
    profile->instructions = 0.0;

    /* For each hotspot in the profile's list of hotspots... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        /* Reseting values */
        hotspot->instructions = 0.0;
        maximum = DBL_MIN;
        minimum = DBL_MAX;
        i = 0;

        /* Calculate total instructions */
        metric = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
        while ((perfexpert_list_item_t *)metric !=
            &(hotspot->metrics.sentinel)) {
            if (0 == strcmp(metric->name, PERFEXPERT_TOOL_HPCTOOLKIT_TOT_INS)) {
                if (maximum < metric->value) {
                    maximum = metric->value;
                }
                if (minimum > metric->value) {
                    minimum = metric->value;
                }
                hotspot->instructions += metric->value;

                i++;
            }
            metric = (metric_t *)perfexpert_list_get_next(metric);
        }

        /* Calculate variance */
        if ((DBL_MIN != maximum) && (DBL_MAX != minimum)) {
            hotspot->variance = (maximum - minimum) / maximum;
            hotspot->instructions = hotspot->instructions / i;
            profile->instructions += hotspot->instructions;
        }

        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    OUTPUT_VERBOSE((5, "    [%d] %s [ins=%g]", profile->id,
        _GREEN(profile->name), profile->instructions));

    /* Calculate importance */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {

        hotspot->importance = hotspot->instructions / profile->instructions;

        OUTPUT_VERBOSE((5, "      [%d] %s [ins=%g] [imp=%g] [var=%g]",
            hotspot->id, _RED(hotspot->name), hotspot->instructions,
            hotspot->importance, hotspot->variance));

        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
