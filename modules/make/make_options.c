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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include "make_options.h"
#include "make.h"

#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"
#include "common/perfexpert_util.h"
#include "install_dirs.h"

static struct argp argp = { options, parse_options, NULL, NULL };
static arg_options_t arg_options = { 0 };

/* parse_cli_params */
int parse_module_args(int argc, char *argv[]) {
    int i = 0;

    /* Set default values */
    arg_options = (arg_options_t) {
        .args            = NULL,
    };

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    if (NULL != arg_options.args) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.args), my_module_globals.args, ' ');
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));

    if (7 <= globals.verbose) {
        printf("\n%s    Arguments:             ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.args[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.args[i]) {
                printf(" [%s]", (char *)my_module_globals.args[i]);
                i++;
            }
        }

        printf("\n");
        fflush(stdout);
    }

    return PERFEXPERT_SUCCESS;
}


/* parse_options */
static error_t parse_options(int key, char *arg, struct argp_state *state) {
    switch (key) {
        /* Should I run some program after each execution? */
        case 'a':
            arg_options.args = arg;
            OUTPUT_VERBOSE((1, "option 'a' set [%s]", arg_options.args));
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
    return PERFEXPERT_SUCCESS;
}

/* module_help */
void module_help(void) {
    argp_help(&argp, stdout, ARGP_HELP_LONG, NULL);
}


#ifdef __cplusplus
}
#endif


