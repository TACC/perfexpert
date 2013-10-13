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
#include <getopt.h>

/* PerfExpert headers */
#include "recommender.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"
#include "install_dirs.h"

/* Structure to handle command line arguments. Try to keep the content of
 * this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"colorful",        no_argument,       NULL, 'c'},
    {"database",        required_argument, NULL, 'd'},
    {"help",            no_argument,       NULL, 'h'},
    {"inputfile",       required_argument, NULL, 'i'},
    {"metricsfile",     required_argument, NULL, 'm'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"pid",             required_argument, NULL, 'p'},
    {"recommendations", required_argument, NULL, 'r'},
    {"verbose",         optional_argument, NULL, 'v'},
    {"workdir",         required_argument, NULL, 'w'},
    {0, 0, 0, 0}
};

/* show_help */
void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: recommender [-cdhimoprvw]\n");
    printf("  -c --colorful        Enable colors on verbose mode, no weird characters will\n");
    printf("                       appear on output files\n");
    printf("  -d --database        Select the recommendation database file\n");
    printf("                       (default: %s/%s)\n", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    printf("  -h --help            Show this message\n");
    printf("  -i --inputfile       Set input file for performance metrics\n");
    printf("  -m --metricsfile     Set input file for user-defined\n");
    printf("  -o --outputfile      Set output file for recommendations (default: STDOUT)\n");
    printf("  -p --pid             Set procedd ID for logging purposes\n");
    printf("  -r --recommendations Number of recommendation to show\n");
    printf("  -v --verbose         Enable verbose mode (default: disabled if not set, 5 if\n");
    printf("                       set, valid options: 1-10)\n");
    printf("  -w --workdir         Directory where output files should be created\n");
    printf("                       (default: current directory)\n");
}

/* parse_env_vars */
int parse_env_vars(void) {
    if ((NULL != getenv("PERFEXPERT_RECOMMENDER_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_RECOMMENDER_COLORFUL")))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE")) {
        globals.dbfile =  getenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE");
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }
    // No variable for -h --help
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_INPUT_FILE")) {
        globals.inputfile = getenv("PERFEXPERT_RECOMMENDER_INPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.inputfile));
    }
    // No variable for -m --metricsfile
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE")) {
        globals.outputfile = getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: outputfile=%s", globals.outputfile));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_PID")) {
        globals.pid = atoi(getenv("PERFEXPERT_RECOMMENDER_PID"));
        OUTPUT_VERBOSE((5, "ENV: pid=%d", globals.pid));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_REC_COUNT")) {
        globals.rec_count = atoi(getenv("PERFEXPERT_RECOMMENDER_REC_COUNT"));
        OUTPUT_VERBOSE((5, "ENV: rec_count=%d", globals.rec_count));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL")) {
        globals.verbose = atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_WORKDIR")) {
        globals.workdir = getenv("PERFEXPERT_RECOMMENDER_WORKDIR");
        OUTPUT_VERBOSE((5, "ENV: workdir=%s", globals.workdir));
    }
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE")) { // Not -m
        globals.outputmetrics = getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE");
        OUTPUT_VERBOSE((5, "ENV: metrics_file=%s", globals.outputmetrics));
    }
    return PERFEXPERT_SUCCESS;
}

/* parse_cli_params */
int parse_cli_params(int argc, char *argv[]) {
    int parameter = 0;    /* Temporary variable to hold parameter */
    int option_index = 0; /* getopt_long() stores the option index here */

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("Error: parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "cd:hi:m:o:p:r:v:w:",
            long_options, &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Activate colorful mode */
            case 'c':
                globals.colorful = PERFEXPERT_TRUE;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;
            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OUTPUT_VERBOSE((10, "option 'd' set [%s]", globals.dbfile));
                break;
            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                return PERFEXPERT_ERROR;
            /* Use input file? */
            case 'i':
                globals.inputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'i' set [%s]", globals.inputfile));
                break;
            /* Specify new metrics */
            case 'm':
                globals.metrics_file = optarg;
                OUTPUT_VERBOSE((10, "option 'm' set [%s]",
                    globals.metrics_file));
                break;
            /* Use output file? */
            case 'o':
                globals.outputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'o' set [%s]", globals.outputfile));
                break;
            /* Specify PID */
            case 'p':
                globals.pid = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'p' set [%d]", globals.pid));
                break;
            /* Number of recommendation to output */
            case 'r':
                globals.rec_count = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'r' set [%d]", globals.rec_count));
                break;
            /* Verbose */
            case 'v':
                globals.verbose = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'v' set [%d]", globals.verbose));
                break;
            /* Workdir */
            case 'w':
                globals.workdir = optarg;
                OUTPUT_VERBOSE((10, "option 'w' set [%s]", globals.workdir));
                break;
            /* Unknown option */
            case '?':
            default:
                show_help();
                return PERFEXPERT_ERROR;
        }
    }

    /* Print summary */
    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((10, "   Colorful verbose?  %s", globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Database file:     %s", globals.dbfile));
    OUTPUT_VERBOSE((10, "   Input file:        %s", globals.inputfile));
    OUTPUT_VERBOSE((10, "   Metrics file:      %s", globals.metrics_file));
    OUTPUT_VERBOSE((10, "   Output file:       %s", globals.outputfile));
    OUTPUT_VERBOSE((10, "   PID:               %d", globals.pid));
    OUTPUT_VERBOSE((10, "   Recommendations:   %d", globals.rec_count));
    OUTPUT_VERBOSE((10, "   Verbose level:     %d", globals.verbose));
    OUTPUT_VERBOSE((10, "   Work directory:    %s", globals.workdir));
    OUTPUT_VERBOSE((10, "   Output metrics:    %s", globals.outputmetrics));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Sanity check: input file is mandatory */
    if (NULL == globals.inputfile) {
        OUTPUT(("%s", _ERROR("Error: undefined input file")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: verbose level should be between 1-10 */
    if ((0 > globals.verbose) || (10 < globals.verbose)) {
        OUTPUT(("%s", _ERROR("Error: invalid verbose level")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
