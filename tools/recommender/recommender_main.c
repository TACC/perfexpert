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
#include <stdio.h>
#include <stdlib.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert tool headers */
#include "recommender.h"

/* PerfExpert common headers */
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    perfexpert_list_t segments;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose          = 0,                 // int
        .outputfile       = NULL,              // char *
        .outputfile_FP    = stdout,            // FILE *
        .outputmetrics    = NULL,              // char *
        .outputmetrics_FP = NULL,              // FILE *
        .dbfile           = NULL,              // char *
        .workdir          = NULL,              // char *
        .uid              = 0,                 // long long int
        .colorful         = PERFEXPERT_FALSE , // int
        .no_rec           = PERFEXPERT_NO_REC, // int
        .rec_count        = 3                  // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Print to a file or STDOUT is? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "   %s (%s)", _YELLOW("printing report to file"),
            globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                globals.outputfile));
            goto CLEANUP;
        }
    } else {
        OUTPUT_VERBOSE((7, "   printing to STDOUT"));
    }

    /* If necessary open outputmetrics_FP */
    if (NULL != globals.outputmetrics) {
        OUTPUT_VERBOSE((7, "   %s (%s)", _YELLOW("printing metrics to file"),
            globals.outputmetrics));
        globals.outputmetrics_FP = fopen(globals.outputmetrics, "w+");
        if (NULL == globals.outputmetrics_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open code transformer "
                "output file"), globals.outputmetrics));
            goto CLEANUP;
        }
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s", _ERROR("Error: connecting to database")));
        goto CLEANUP;
    }

    /* Select recommendations */
    if (PERFEXPERT_SUCCESS != select_recommendations()) {
        OUTPUT(("%s", _ERROR("Error: selecting recommendations")));
        goto CLEANUP;
    }

    CLEANUP:
    /* Close input file */
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    if (NULL != globals.outputmetrics) {
        fclose(globals.outputmetrics_FP);
    }
    perfexpert_database_disconnect(globals.db);

    return globals.no_rec;
}

#ifdef __cplusplus
}
#endif

// EOF
