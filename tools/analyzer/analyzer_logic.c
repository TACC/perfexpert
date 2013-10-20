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
    module_t *module = NULL, *module_temp = NULL;
    metric_t *metric = NULL;
    double maximum = DBL_MIN, minimum = DBL_MAX;
    int count = 0;

    OUTPUT_VERBOSE((4, "   %s", _CYAN("Calculating importance and variance")));

    /* For each hotspot in the profile's list of hotspots... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        /* Reseting values */
        hotspot->instructions = 0.0;
        hotspot->cycles = 0.0;
        maximum = DBL_MIN;
        minimum = DBL_MAX;
        count = 0;

        /* Calculate total instructions */
        metric = (metric_t *)perfexpert_list_get_first(&(hotspot->metrics));
        while ((perfexpert_list_item_t *)metric !=
            &(hotspot->metrics.sentinel)) {
            if (0 == strcmp(metric->name,
                globals.toolmodule.total_instructions)) {
                if (maximum < metric->value) {
                    maximum = metric->value;
                }
                if (minimum > metric->value) {
                    minimum = metric->value;
                }
                hotspot->instructions += metric->value;
                count++;
            }
            if (0 == strcmp(metric->name, globals.toolmodule.total_cycles)) {
                hotspot->cycles += metric->value;
                profile->cycles += metric->value;

                if (PERFEXPERT_HOTSPOT_LOOP == hotspot->type) {
                    loop_t *loop = (loop_t *)hotspot;
                    loop->procedure->module->cycles += metric->value;
                }
                if (PERFEXPERT_HOTSPOT_FUNCTION == hotspot->type) {
                    hotspot->module->cycles += metric->value;
                }
            }
            metric = (metric_t *)perfexpert_list_get_next(metric);
        }

        /* Calculate variance */
        if ((DBL_MIN != maximum) && (DBL_MAX != minimum)) {
            hotspot->variance = (maximum - minimum) / maximum;
            hotspot->instructions = hotspot->instructions / count;
            profile->instructions += hotspot->instructions;
        }

        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    /* Calculate importance for hotspots */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {

        hotspot->importance = hotspot->cycles / profile->cycles;

        OUTPUT_VERBOSE((5,
            "    [%d] %s [ins=%g] [var=%g] [cyc=%g] [importance=%.2f%%]",
            hotspot->id, _YELLOW(hotspot->name), hotspot->instructions,
            hotspot->variance, hotspot->cycles, hotspot->importance * 100));

        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }

    /* Calculate importance for modules */
    perfexpert_hash_iter_int(profile->modules_by_id, module, module_temp) {
        module->importance = module->cycles / profile->cycles;

        OUTPUT_VERBOSE((5, "    [%d] %s [cyc=%g] [importance=%.2f%%]",
            module->id, _YELLOW(module->name), module->cycles,
            module->importance * 100));
    }

    OUTPUT_VERBOSE((5, "   %s [ins=%g] [cyc=%g]", _MAGENTA("total"),
        profile->instructions, profile->cycles));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
