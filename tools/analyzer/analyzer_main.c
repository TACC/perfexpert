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
    
/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "install_dirs.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char **argv) {
    perfexpert_list_t profiles;

    /* Set default values for globals */
    globals = (globals_t) {
        .threshold       = 0.0,              // double
        .tool            = "hpctoolkit",     // char *
        .inputfile       = NULL,             // char *
        .outputfile      = NULL,             // char *
        .outputfile_FP   = stdout,           // char *
        .aggregate       = PERFEXPERT_FALSE, // int
        .thread          = -1,               // int
        .machinefile     = NULL,             // char *
        .machine_by_name = NULL,             // metric_t *
        .lcpifile        = NULL,             // char *
        .lcpi_by_name    = NULL,             // lcpi_t *
        .verbose         = 0,                // int
        .colorful        = PERFEXPERT_FALSE, // int
        .order           = "none",           // char *
        .outputmetrics   = NULL              // char *
    };

    /* Initialize list of profiles */
    perfexpert_list_construct(&profiles);

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Set default values */
    if (NULL == globals.machinefile) {
        PERFEXPERT_ALLOC(char, globals.machinefile,
            (strlen(PERFEXPERT_ETCDIR) + strlen(MACHINE_FILE) + 2));
        sprintf(globals.machinefile, "%s/%s", PERFEXPERT_ETCDIR, MACHINE_FILE);
    }
    if (NULL == globals.lcpifile) {
        PERFEXPERT_ALLOC(char, globals.lcpifile,
            (strlen(PERFEXPERT_ETCDIR) + strlen(LCPI_FILE) + 2));
        sprintf(globals.lcpifile, "%s/%s", PERFEXPERT_ETCDIR, LCPI_FILE);
    }

    /* Print to a file or STDOUT? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "   %s (%s)",
            _YELLOW("printing recommendations to file"), globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                globals.outputfile));
            return PERFEXPERT_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((7, "   printing recommendations to STDOUT"));
    }

    /* Parse LCPI metrics */
    if (PERFEXPERT_SUCCESS != lcpi_parse_file(globals.lcpifile)) {
        OUTPUT(("%s (%s)", _ERROR("Error: LCPI file is not valid"),
            globals.lcpifile));
        return PERFEXPERT_ERROR;
    }

    /* Parse machine characterization */
    if (PERFEXPERT_SUCCESS != machine_parse_file(globals.machinefile)) {
        OUTPUT(("%s (%s)",
            _ERROR("Error: Machine characterization file is not valid"),
            globals.machinefile));
        return PERFEXPERT_ERROR;
    }

    /* Parse input file */
    if (PERFEXPERT_SUCCESS != profile_parse_file(globals.inputfile,
        globals.tool, &profiles)) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to parse input file"),
            globals.inputfile));
        return PERFEXPERT_ERROR;
    }

    /* Check and flatten profiles */
    if (PERFEXPERT_SUCCESS != profile_check_all(&profiles)) {
        OUTPUT(("%s", _ERROR("Error: checking profile")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != profile_flatten_all(&profiles)) {
        OUTPUT(("%s (%s)", _ERROR("Error: flatening profiles"),
            globals.inputfile));
        return PERFEXPERT_ERROR;
    }

    /* Sort hotspots */
    if (NULL != globals.order) {
        hotspot_sort(&profiles);
    }

    /* Output analysis report */
    if (PERFEXPERT_SUCCESS != output_analysis_all(&profiles)) {
        OUTPUT(("%s", _ERROR("Error: printing profiles analysis")));
        return PERFEXPERT_ERROR;
    }

    /* Output list of metrics per hotspot (input for perfexpert_recommender) */
    if (NULL != globals.outputmetrics) {
        if (PERFEXPERT_SUCCESS != output_metrics_all(&profiles)) {
            OUTPUT(("%s", _ERROR("Error: printing profiles metrics")));
            return PERFEXPERT_ERROR;
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

// EOF
