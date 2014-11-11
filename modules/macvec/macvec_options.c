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
// #include <string.h>
#include <argp.h>

/* Modules headers */
#include "macvec.h"
#include "macvec_options.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

static struct argp argp = { options, parse_options, NULL, NULL };

/* parse_cli_params */
int parse_module_args(int argc, char *argv[]) {
    int i = 0;

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Set default threshold value */
    my_module_globals.threshold = 0.01;

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Sanity check: threshold is mandatory, check limits */
    if (((0 >= my_module_globals.threshold) ||
        (1 < my_module_globals.threshold))) {
        OUTPUT(("%s", _ERROR("invalid threshold")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Threshold:     %f", my_module_globals.threshold));
    OUTPUT_VERBOSE((7, "   Architecture:  %s", my_module_globals.architecture));
    OUTPUT_VERBOSE((7, "   Verbose level: %d", my_module_globals.verbose));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= my_module_globals.verbose) {
        i = 0;
        printf("%s %s", PROGRAM_PREFIX, _YELLOW("options:"));
        for (i = 0; i < argc; i++) {
            printf(" [%s]", argv[i]);
        }
        printf("\n");
        fflush(stdout);
    }

    return PERFEXPERT_SUCCESS;
}

/* parse_options */
static error_t parse_options(int key, char *arg, struct argp_state *state) {
    switch (key) {
        /* Sorting order */
        case 'a':
            my_module_globals.architecture = arg;
            OUTPUT_VERBOSE((1, "option 'a' set [%s]",
                my_module_globals.architecture));
            break;

        /* Threshold */
        case 't':
            my_module_globals.threshold = atof(arg);
            OUTPUT_VERBOSE((1, "option 't' set [%f]",
                my_module_globals.threshold));
            break;

        /* Verbose */
        case 'v':
            my_module_globals.verbose = atoi(arg);
            OUTPUT_VERBOSE((1, "option 'v' set [%f]",
                my_module_globals.verbose));
            break;

        /* no arguments... */
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
    if (NULL != getenv("PERFEXPERT_MODULE_LCPI_THRESHOLD")) {
        my_module_globals.threshold = atof(
            getenv("PERFEXPERT_MODULE_LCPI_THRESHOLD"));
        OUTPUT_VERBOSE((1, "ENV: threshold=%f", my_module_globals.threshold));
    }

    if (NULL != getenv("PERFEXPERT_MODULE_LCPI_ARCHITECTURE")) {
        my_module_globals.architecture =
            getenv("PERFEXPERT_MODULE_LCPI_ARCHITECTURE");
        OUTPUT_VERBOSE((1, "ENV: order=%s", my_module_globals.architecture));
    }

    return PERFEXPERT_SUCCESS;
}

/* module_help */
void module_help(void) {
    argp_help(&argp, stdout, ARGP_HELP_LONG, NULL);
}

#ifdef __cplusplus
}
#endif

// EOF
