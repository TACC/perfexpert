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

#include "macpo_options.h"
#include "macpo.h"
#include "macpo_types.h"


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
        .prefix            = NULL,
        .before            = NULL,
        .after             = NULL,
    };


    OUTPUT(("%s -- %d -- %s", _ERROR("parsing environment variables"), argc, argv[0]));

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT(("%s", _ERROR("parsing arguments")));
    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    OUTPUT(("%s", _ERROR("Expanding")));

    /* Expand AFTERs, BEFOREs, and PREFIXs arguments */
    // TODO: right now we only use the before, after and
    // prefix from globals. We should also consider values coming
    // throught args_options.* (that have been set in the previous
    // instruction
    //args_options.after = globals.after;
    if (NULL != arg_options.after) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.after), my_module_globals.after, ' ');
    }
    //args_options.before = globals.before;
    if (NULL != arg_options.before) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.before), my_module_globals.before, ' ');
    }
    //arg_options.prefix = globals.prefix;
    if (NULL != arg_options.prefix) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.prefix), my_module_globals.prefix, ' ');
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Ignore return code:  %s",
        my_module_globals.ignore_return_code ? "yes" : "no"));
//    OUTPUT_VERBOSE((7, "   Program input file:  %s",
//        my_module_globals.inputfile));

    if (7 <= globals.verbose) {
        printf("%s    Prefix:             ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.prefix[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.prefix[i]) {
                printf(" [%s]", (char *)my_module_globals.prefix[i]);
                i++;
            }
        }

        printf("\n%s    Before each run:    ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.before[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.before[i]) {
                printf(" [%s]", (char *)my_module_globals.before[i]);
                i++;
            }
        }

        printf("\n%s    After each run:     ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.after[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.after[i]) {
                printf(" [%s]", (char *)my_module_globals.after[i]);
                i++;
            }
        }
        printf("\n");
        fflush(stdout);
    }

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
    printf ("option %c", key);
    switch (key) {
        /* Should I run some program after each execution? */
        case 'a':
            arg_options.after = arg;
            OUTPUT_VERBOSE((1, "option 'a' set [%s]", arg_options.after));
            break;

        /* Should I run some program before each execution? */
        case 'b':
            arg_options.before = arg;
            OUTPUT_VERBOSE((1, "option 'b' set [%s]", arg_options.before));
            break;

        /* Should I add a program prefix to the command line? */
        case 'p':
            arg_options.prefix = arg;
            OUTPUT_VERBOSE((1, "option 'p' set [%s]", arg_options.prefix));
            break;

        /* Should I ignore the target return code? */
        case 'r':
            my_module_globals.ignore_return_code = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'r' set"));
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
    if (NULL != getenv("PERFEXPERT_MACPO_PREFIX")) {
        arg_options.prefix = ("PERFEXPERT_MACPO_PREFIX");
        OUTPUT_VERBOSE((1, "ENV: prefix=%s", arg_options.prefix));
    }

    if (NULL != getenv("PERFEXPERT_MACPO_BEFORE")) {
        arg_options.before = ("PERFEXPERT_MACPO_BEFORE");
        OUTPUT_VERBOSE((1, "ENV: before=%s", arg_options.before));
    }

    if (NULL != getenv("PERFEXPERT_MACPO_AFTER")) {
        arg_options.after = ("PERFEXPERT_MACPO_AFTER");
        OUTPUT_VERBOSE((1, "ENV: after=%s", arg_options.after));
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


