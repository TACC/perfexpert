/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
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
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_output.h"
#include "perfexpert_list.h"
#include "perfexpert_database.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    perfexpert_list_t *segments;
    segment_t *segment;
    function_t *function;
    int rc = PERFEXPERT_NO_REC;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level    = 0,              // int
        .inputfile        = NULL,           // char *
        .inputfile_FP     = stdin,          // FILE *
        .outputfile       = NULL,           // char *
        .outputfile_FP    = stdout,         // FILE *
        .dbfile           = NULL,           // char *
        .workdir          = NULL,           // char *
        .pid              = (long)getpid(), // long int
        .colorful         = 0,              // int
        .metrics_file     = NULL,           // char *
        .use_temp_metrics = 0,              // int
        .metrics_table    = METRICS_TABLE,  // char *
        .rec_count        = 3               // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create the list of code bottlenecks */
    segments = (perfexpert_list_t *)malloc(sizeof(perfexpert_list_t));
    if (NULL == segments) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_construct(segments);
    
    /* Open input file */
    if (NULL != globals.inputfile) {
        if (NULL == (globals.inputfile_FP = fopen(globals.inputfile, "r"))) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open input file"),
                globals.inputfile));
            return PERFEXPERT_ERROR;
        }
    }

    /* Print to a file or STDOUT is? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "   %s (%s)",
            _YELLOW("printing recommendations to file"), globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                globals.outputfile));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }
    } else {
        OUTPUT_VERBOSE((7, "   printing recommendations to STDOUT"));
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s", _ERROR("Error: connecting to database")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }
    
    /* Parse metrics file if 'm' is defined, this will create a temp table */
    if (1 == globals.use_temp_metrics) {
        globals.metrics_table = (char *)malloc(strlen("metrics_") + 6);
        sprintf(globals.metrics_table, "metrics_%d", (int)getpid());

        if (PERFEXPERT_SUCCESS != parse_metrics_file()) {
            OUTPUT(("%s", _ERROR("Error: parsing metrics file")));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }
    }
    
    /* Parse input parameters */
    if (PERFEXPERT_SUCCESS != parse_segment_params(segments)) {
        OUTPUT(("%s", _ERROR("Error: parsing input params")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }

    /* Select recommendations for each code bottleneck */
    segment = (segment_t *)perfexpert_list_get_first(segments);
    while ((perfexpert_list_item_t *)segment != &(segments->sentinel)) {
        OUTPUT_VERBOSE((4, "%s (%s:%d)",
            _YELLOW("selecting recommendation for"), segment->filename,
            segment->line_number));

        /* Query DB for recommendations */
        switch (select_recommendations(segment)) {
            case PERFEXPERT_NO_REC:
                OUTPUT(("%s", _GREEN("Sorry, we have no recommendations")));
                /* Move to the next code bottleneck */
                segment = (segment_t *)perfexpert_list_get_next(segment);
                continue;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            default: 
                OUTPUT(("%s", _ERROR("Error: selecting recommendations")));
                rc = PERFEXPERT_ERROR;
                goto cleanup;
        }
        /* Move to the next code bottleneck */
        segment = (segment_t *)perfexpert_list_get_next(segment);
    }

    cleanup:
    /* Close input file */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    perfexpert_database_disconnect(globals.db);

    /* TODO: Free memory */

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
