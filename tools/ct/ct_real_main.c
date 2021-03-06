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
#include "ct.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int ct_main(int argc, char** argv) {
    perfexpert_list_t fragments;
    fragment_t *fragment = NULL;
    int rc = PERFEXPERT_NO_TRANS;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose       = 0,               // int
        .inputfile     = NULL,            // char *
        .inputfile_FP  = stdin,           // FILE *
        .outputfile    = NULL,            // char *
        .outputfile_FP = stdout,          // FILE *
        .dbfile        = NULL,            // char *
        .db            = NULL,            // sqlite3 *
        .workdir       = NULL,            // char *
        .pid           = (long)getpid(),  // long int
        .colorful      = PERFEXPERT_FALSE // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create the list of fragments */
    perfexpert_list_construct(&fragments);

    /* Open input file */
    if (NULL != globals.inputfile) {
        if (NULL == (globals.inputfile_FP = fopen(globals.inputfile, "r"))) {
            OUTPUT(("%s (%s)", _ERROR("unable to open input file"),
                globals.inputfile));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((3, "using (%s) as input for perf measurements",
            globals.inputfile));
    } else {
        OUTPUT_VERBOSE((3, "using STDIN as input for perf measurements"));
    }

    /* Print to a file or STDOUT is? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "printing transformations to file (%s)",
            globals.outputfile));
        if (NULL == (globals.outputfile_FP = fopen(globals.outputfile, "w+"))) {
            OUTPUT(("%s (%s)", _ERROR("unable to open output file"),
                globals.outputfile));
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }
    } else {
        OUTPUT_VERBOSE((7, "printing transformations to STDOUT"));
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s", _ERROR("connecting to database")));
        rc = PERFEXPERT_ERROR;
        goto CLEAN_UP;
    }

    /* Parse input parameters */
    if (PERFEXPERT_SUCCESS != parse_transformation_params(&fragments)) {
        OUTPUT(("%s", _ERROR("parsing input params")));
        rc = PERFEXPERT_ERROR;
        goto CLEAN_UP;
    }

    /* Select transformations for each code bottleneck */
    fragment = (fragment_t *)perfexpert_list_get_first(&fragments);
    while ((perfexpert_list_item_t *)fragment != &(fragments.sentinel)) {
        /* Query DB for transformations */
        if (PERFEXPERT_SUCCESS != select_transformations(fragment)) {
            OUTPUT(("%s", _ERROR("applying transformation")));
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }

        /* Apply recommendation (actually only one will be applied) */
        switch (apply_recommendations(fragment)) {
            case PERFEXPERT_NO_TRANS:
                OUTPUT(("%s", _GREEN("Sorry, we have no transformations")));
                goto MOVE_ON;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            default:
                OUTPUT(("%s", _ERROR("selecting transformations")));
                rc = PERFEXPERT_ERROR;
                goto CLEAN_UP;
        }

        /* Move to the next code bottleneck */
        MOVE_ON:
        fragment = (fragment_t *)perfexpert_list_get_next(fragment);
    }

    CLEAN_UP:
    /* Close files and DB connection */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    perfexpert_database_disconnect(globals.db);

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
