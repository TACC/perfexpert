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

/* Modules headers */
#include "gcc.h"
#include "gcc_options.h"

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

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Sanity check: source file is mandatory, check if it exists */
    if ((PERFEXPERT_ERROR ==
        perfexpert_util_file_exists(my_module_globals.source)) &&
        (PERFEXPERT_FALSE == my_module_globals.help_only)) {
        OUTPUT(("%s", _ERROR("source file not found")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Compiler: %s", my_module_globals.compiler));
    OUTPUT_VERBOSE((7, "   Source:   %s", my_module_globals.source));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
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
        /* Compiler full path */
        case 'p':
            my_module_globals.compiler = arg;
            OUTPUT_VERBOSE((1, "option 'p' set [%s]",
                my_module_globals.compiler));
            break;

        /* Source file */
        case 's':
            my_module_globals.source = arg;
            OUTPUT_VERBOSE((1, "option 's' set [%s]",
                my_module_globals.source));
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
    if (NULL != getenv("PERFEXPERT_MODULE_GCC_SOURCE")) {
        my_module_globals.source = getenv("PERFEXPERT_MODULE_GCC_SOURCE");
        OUTPUT_VERBOSE((1, "ENV: source=%f", my_module_globals.source));
    }

    if (NULL != getenv("PERFEXPERT_MODULE_GCC_FULL_PATH")) {
        my_module_globals.compiler = getenv("PERFEXPERT_MODULE_GCC_FULL_PATH");
        OUTPUT_VERBOSE((1, "ENV: source=%f", my_module_globals.compiler));
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
