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
#include "perfexpert_alloc.h"
#include "perfexpert_database.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    perfexpert_list_t segments;
    int rc = PERFEXPERT_ERROR;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose          = 0,                // int
        .inputfile        = NULL,             // char *
        .inputfile_FP     = NULL,             // FILE *
        .outputfile       = NULL,             // char *
        .outputfile_FP    = stdout,           // FILE *
        .outputmetrics    = NULL,             // char *
        .outputmetrics_FP = NULL,             // FILE *
        .dbfile           = NULL,             // char *
        .workdir          = NULL,             // char *
        .pid              = (long)getpid(),   // long int
        .colorful         = PERFEXPERT_FALSE, // int
        .metrics_file     = NULL,             // char *
        .metrics_table    = METRICS_TABLE,    // char *
        .rec_count        = 3                 // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create the list of code bottlenecks */
    perfexpert_list_construct(&segments);
    
    /* Open input file */
    OUTPUT_VERBOSE((3, "   %s (%s)", _YELLOW("performance analysis input"),
        globals.inputfile));
    if (NULL == (globals.inputfile_FP = fopen(globals.inputfile, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open input file"),
            globals.inputfile));
        return PERFEXPERT_ERROR;
    }

    /* Print to a file or STDOUT is? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "   %s (%s)", _YELLOW("printing report to file"),
            globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open report file"),
                globals.outputfile));
            goto CLEANUP;
        }
    } else {
        OUTPUT_VERBOSE((7, "   printing report to STDOUT"));
    }

    /* If necessary open outputmetrics_FP */
    if (NULL != globals.outputmetrics) {
        OUTPUT_VERBOSE((7, "   %s (%s)", _YELLOW("printing metrics to file"),
            globals.outputmetrics));
        globals.outputmetrics_FP = fopen(globals.outputmetrics, "w+");
        if (NULL == globals.outputmetrics_FP) {
            OUTPUT(("%s (%s)",
                _ERROR("Error: unable to open output metrics file"),
                globals.outputmetrics));
            goto CLEANUP;
        }
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s", _ERROR("Error: connecting to database")));
        goto CLEANUP;
    }
    
    /* Parse metrics file if 'm' is defined, this will create a temp table */
    if (NULL != globals.metrics_file) {
        PERFEXPERT_ALLOC(char, globals.metrics_table, (strlen("metrics_") + 6));
        sprintf(globals.metrics_table, "metrics_%d", (int)getpid());

        if (PERFEXPERT_SUCCESS != parse_metrics_file()) {
            OUTPUT(("%s", _ERROR("Error: parsing metrics file")));
            goto CLEANUP;
        }
    }
    
    /* Parse input parameters */
    if (PERFEXPERT_SUCCESS != parse_segment_params(&segments)) {
        OUTPUT(("%s", _ERROR("Error: parsing input params")));
        goto CLEANUP;
    }

    /* Select recommendations */
    rc = select_recommendations_all(&segments);

    CLEANUP:
    /* Close input file */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    if (NULL != globals.outputmetrics) {
        fclose(globals.outputmetrics_FP);
    }
    perfexpert_database_disconnect(globals.db);

    /* Free memory */
    if (NULL != globals.metrics_file) {
        PERFEXPERT_DEALLOC(globals.metrics_table);
    }

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
