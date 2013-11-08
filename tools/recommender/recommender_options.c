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
#include <argp.h>
#include <math.h>

/* PerfExpert headers */
#include "recommender.h"
#include "recommender_options.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

static struct argp argp = { options, parse_options, args_doc, doc };

/* parse_cli_params */
int parse_cli_params(int argc, char *argv[]) {
    int i = 0;

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, &globals);

    /* Sanity check: verbose level should be between 1-10 */
    if ((0 > globals.verbose) || (10 < globals.verbose)) {
        OUTPUT(("%s", _ERROR("invalid verbose level")));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: unique ID is mandatory */
    if (0 == globals.uid) {
        OUTPUT(("%s", _ERROR("undefined uid")));
        return PERFEXPERT_ERROR;
    }

    /* Print summary */
    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Verbose level:      %d", globals.verbose));
    OUTPUT_VERBOSE((7, "   Colorful verbose?   %s", globals.colorful ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   Database file:      %s", globals.dbfile));
    OUTPUT_VERBOSE((7, "   UID:                %llu", globals.uid));
    OUTPUT_VERBOSE((7, "   Recommendations:    %d", globals.rec_count));
    OUTPUT_VERBOSE((7, "   Output file:        %s", globals.outputfile));
    OUTPUT_VERBOSE((7, "   Output transformer: %s", globals.outputmetrics));
    OUTPUT_VERBOSE((7, "   Work directory:     %s", globals.workdir));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (7 <= globals.verbose) {
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    return PERFEXPERT_SUCCESS;
}

/* parse_options */
static error_t parse_options(int key, char *arg, struct argp_state *state) {
    switch (key) {

        /* Activate colorful mode */
        case 'c':
            globals.colorful = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'c' set"));
            break;

        /* Database */
        case 'd':
            globals.dbfile = arg;
            OUTPUT_VERBOSE((1, "option 'd' set [%s]", globals.dbfile));
            break;

        /* Show help */
        case 'h':
            OUTPUT_VERBOSE((1, "option 'h' set"));
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
            break;

        /* Verbose level (has an alias: v) */
        case 'l':
            globals.verbose = arg ? atoi(arg) : 5;
            OUTPUT_VERBOSE((1, "option 'l' set [%d]", globals.verbose));
            break;

        /* Output file */
        case 'o':
            globals.outputfile = arg;
            OUTPUT_VERBOSE((1, "option 'o' set [%s]", globals.outputfile));
            break;

        /* Number of recommendation to output */
        case 'r':
            globals.rec_count = atoi(arg);
            OUTPUT_VERBOSE((1, "option 'r' set [%d]", globals.rec_count));
            break;

        /* Output for code transformer */
        case 't':
            globals.outputmetrics = arg;
            OUTPUT_VERBOSE((1, "option 't' set [%d]", globals.outputmetrics));
            break;

        /* Unique ID */
        case 'u':
            globals.uid = strtoll(arg, NULL, 10);
            OUTPUT_VERBOSE((1, "option 't' set [%llu]", globals.uid));
            break;

        /* Verbose level (has an alias: l) */
        case 'v':
            globals.verbose = arg ? atoi(arg) : 5;
            OUTPUT_VERBOSE((1, "option 'v' set [%d]", globals.verbose));
            break;

        /* Workdir */
        case 'w':
            globals.workdir = optarg;
            OUTPUT_VERBOSE((10, "option 'w' set [%s]", globals.workdir));
            break;

        /* ... */
        case ARGP_KEY_ARG:
        case ARGP_KEY_NO_ARGS:
        case ARGP_KEY_END:
            break;

        /* Unknown option */
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* parse_env_vars */
static int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL")) {
        globals.verbose = atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose));
    }

    if ((NULL != getenv("PERFEXPERT_RECOMMENDER_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_RECOMMENDER_COLORFUL")))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_DATABASE")) {
        globals.dbfile =  getenv("PERFEXPERT_RECOMMENDER_DATABASE");
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE")) {
        globals.outputfile = getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: outputfile=%s", globals.outputfile));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_UID")) {
        globals.uid = strtoll(getenv("PERFEXPERT_RECOMMENDER_UID"), NULL, 10);
        OUTPUT_VERBOSE((5, "ENV: uid=%llu", globals.uid));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_REC_COUNT")) {
        globals.rec_count = atoi(getenv("PERFEXPERT_RECOMMENDER_REC_COUNT"));
        OUTPUT_VERBOSE((5, "ENV: rec_count=%d", globals.rec_count));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_WORKDIR")) {
        globals.workdir = getenv("PERFEXPERT_RECOMMENDER_WORKDIR");
        OUTPUT_VERBOSE((5, "ENV: workdir=%s", globals.workdir));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE")) {
        globals.outputmetrics = getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE");
        OUTPUT_VERBOSE((5, "ENV: metrics_file=%s", globals.outputmetrics));
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
