/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>

/* Utility headers */
#include <matheval.h>

/* Modules headers */
#include "lcpi.h"
#include "lcpi_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* logic_lcpi_compute */
int logic_lcpi_compute(lcpi_profile_t *profile) {

    lcpi_metric_t *h_lcpi = NULL, *l = NULL, *t = NULL;
    lcpi_hotspot_t *h = NULL;
    double *values = NULL;
    lcpi_hound_t *hound_info, *ddd;
    char **names = NULL;
    int count = 0, i = 0;
    int mpi_tasks, threads;
    int task, thread;

    OUTPUT_VERBOSE((4, "%s", _YELLOW("Calculating LCPI metrics")));

    if (my_module_globals.hound_info==NULL) {
        if (PERFEXPERT_SUCCESS != import_hound (my_module_globals.hound_info)) {
            OUTPUT((_ERROR("importing hound")));
            return PERFEXPERT_ERROR;
        }
    }

    mpi_tasks = database_get_mpi_tasks();
    threads = database_get_threads();

    /* For each hotspot in this profile... */
    perfexpert_list_for(h, &(profile->hotspots), lcpi_hotspot_t) {
        OUTPUT_VERBOSE((10, "  %s (%s:%d@%s)", _YELLOW(h->name), 
            h->file, h->line, h->module->name));

        /* For each LCPI definition... */
        perfexpert_hash_iter_str(my_module_globals.metrics_by_name, l, t) {
            /* Get the list of variables and their values */
            evaluator_get_variables(l->expression, &names, &count);
            if (count <= 0) {
                continue;
            }
            /* Iterate over all the MPI processes */
            for (task = 0; task < mpi_tasks; task++) {
                /* Iterate over all threads */
                for (thread = 0; thread < threads; thread++) {
                    /* Allocate the metric */
                    PERFEXPERT_ALLOC(lcpi_metric_t, h_lcpi, sizeof(lcpi_metric_t));
                    strcpy(h_lcpi->name_md5, l->name_md5);
                    h_lcpi->expression = l->expression;
                    h_lcpi->value = l->value;
                    h_lcpi->name = l->name;
                    h_lcpi->mpi_task = task;
                    h_lcpi->thread_id = thread;

                    PERFEXPERT_ALLOC(double, values, (sizeof(double *) * count));
                    /* Iterate over all the events of each metric */
                    for (i = 0; i < count; i++) {
                        /* Check if the current event is in hound */
                        perfexpert_hash_find_str(my_module_globals.hound_info, perfexpert_md5_string(names[i]), hound_info);
                        if (hound_info) {
                            /* If in hound, use that value */
                            values[i] = hound_info->value;
                            OUTPUT_VERBOSE((10, "           Found name %s = %g", names[i], values[i]));
                        } else {
                            /* If not in hound, look that event in the DB for the task and thread */
                            values[i] = database_get_event(names[i], h->id, task, thread);
                            if (values[i] != -1.0) {
                                OUTPUT_VERBOSE((10, "      [%d] Found name %s = %g", h->id, names[i], values[i]));
                            }
                            else {
                                /* Value not found */
                                values[i] = 0.0;
                            }
                        }
                    }
                    /* Evaluate the LCPI expression */
                    h_lcpi->value = evaluator_evaluate(h_lcpi->expression, count,
                                                       names, values);
                    /* Add the LCPI to the hotspot's list of LCPIs */
                    perfexpert_hash_add_str(h->metrics_by_name, name_md5, h_lcpi);

                    OUTPUT_VERBOSE((10, "    %s (%d - %d) = [%g]", h_lcpi->name, h_lcpi->mpi_task, h_lcpi->thread_id,  h_lcpi->value));
                    PERFEXPERT_DEALLOC(values);
                    // if it's serial (aggregated), we don't need to continue
                    if (my_module_globals.output == SERIAL_OUTPUT) 
                        break;
                }
                // if it's serial (aggregated), we don't need to continue
                if (my_module_globals.output == SERIAL_OUTPUT)
                    break;
            }
        }
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
