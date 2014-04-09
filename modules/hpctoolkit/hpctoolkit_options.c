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
#include <string.h>
#include <argp.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_options.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"
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
        .mic_prefix        = NULL,
        .mic_before        = NULL,
        .mic_after         = NULL
    };

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Expand AFTERs, BEFOREs, and PREFIXs arguments */
    if (NULL != arg_options.after) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.after), my_module_globals.after, ' ');
    }
    if (NULL != arg_options.before) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.before), my_module_globals.before, ' ');
    }
    if (NULL != arg_options.prefix) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.prefix), my_module_globals.prefix, ' ');
    }
    if (NULL != arg_options.mic_after) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.mic_after), my_module_globals.mic_after, ' ');
    }
    if (NULL != arg_options.mic_before) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.mic_before), my_module_globals.mic_before, ' ');
    }
    if (NULL != arg_options.mic_prefix) {
        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.mic_prefix), my_module_globals.mic_prefix, ' ');
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != my_module_globals.mic_prefix[0]) &&
        (NULL == my_module_globals.mic)) {
        OUTPUT(("%s option -P selected but no MIC card was specified, ignoring",
            _BOLDRED("WARNING:")));
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != my_module_globals.mic_before[0]) &&
        (NULL == my_module_globals.mic)) {
        OUTPUT(("%s option -B selected but no MIC card was specified, ignoring",
            _BOLDRED("WARNING:")));
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != my_module_globals.mic_after[0]) &&
        (NULL == my_module_globals.mic)) {
        OUTPUT(("%s option -A selected but no MIC card was specified, ignoring",
            _BOLDRED("WARNING:")));
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Program input file:  %s",
        my_module_globals.inputfile));

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

    OUTPUT_VERBOSE((7, "   MIC card:            %s", my_module_globals.mic));

    if (7 <= globals.verbose) {
        printf("%s    MIC prefix:         ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.mic_prefix[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.mic_prefix[i]) {
                printf(" [%s]", (char *)my_module_globals.mic_prefix[i]);
                i++;
            }
        }

        printf("\n%s    MIC before each run:", PROGRAM_PREFIX);
        if (NULL == my_module_globals.mic_before[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.mic_before[i]) {
                printf(" [%s]", (char *)my_module_globals.mic_before[i]);
                i++;
            }
        }

        printf("\n%s    MIC after each run: ", PROGRAM_PREFIX);
        if (NULL == my_module_globals.mic_after[0]) {
            printf(" (null)");
        } else {
            i = 0;
            while (NULL != my_module_globals.mic_after[i]) {
                printf(" [%s]", (char *)my_module_globals.mic_after[i]);
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
    switch (key) {
        /* Should I run some program after each execution? */
        case 'a':
            arg_options.after = arg;
            OUTPUT_VERBOSE((1, "option 'a' set [%s]", arg_options.after));
            break;

        /* Should I run on the MIC some program after each execution? */
        case 'A':
            arg_options.mic_after = arg;
            OUTPUT_VERBOSE((1, "option 'A' set [%s]", arg_options.mic_after));
            break;

        /* Should I run some program before each execution? */
        case 'b':
            arg_options.before = arg;
            OUTPUT_VERBOSE((1, "option 'b' set [%s]", arg_options.before));
            break;

        /* Should I run on the MIC some program before each execution? */
        case 'B':
            arg_options.mic_before = arg;
            OUTPUT_VERBOSE((1, "option 'B' set [%s]", arg_options.mic_before));
            break;

        /* MIC card */
        case 'C':
            my_module_globals.mic = arg;
            OUTPUT_VERBOSE((1, "option 'C' set [%s]", my_module_globals.mic));
            break;

        /* Help */
        case 'h':
            OUTPUT_VERBOSE((1, "option 'h' set"));
            argp_help(&argp, stdout, ARGP_HELP_LONG, NULL);
            break;

        /* Which input file? */
        case 'i':
            my_module_globals.inputfile = arg;
            OUTPUT_VERBOSE((1, "option 'i' set [%s]",
                my_module_globals.inputfile));
            break;

        /* Should I add a program prefix to the command line? */
        case 'p':
            arg_options.prefix = arg;
            OUTPUT_VERBOSE((1, "option 'p' set [%s]", arg_options.prefix));
            break;

        /* Should I add a program prefix to the MIC command line? */
        case 'P':
            arg_options.mic_prefix = arg;
            OUTPUT_VERBOSE((1, "option 'P' set [%s]", arg_options.mic_prefix));
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
    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_PREFIX")) {
        arg_options.prefix = ("PERFEXPERT_HPCTOOLKIT_PREFIX");
        OUTPUT_VERBOSE((1, "ENV: prefix=%s", arg_options.prefix));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_BEFORE")) {
        arg_options.before = ("PERFEXPERT_HPCTOOLKIT_BEFORE");
        OUTPUT_VERBOSE((1, "ENV: before=%s", arg_options.before));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_AFTER")) {
        arg_options.after = ("PERFEXPERT_HPCTOOLKIT_AFTER");
        OUTPUT_VERBOSE((1, "ENV: after=%s", arg_options.after));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_MIC_CARD")) {
        my_module_globals.mic = ("PERFEXPERT_HPCTOOLKIT_MIC_CARD");
        OUTPUT_VERBOSE((1, "ENV: mic=%s", my_module_globals.mic));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_MIC_PREFIX")) {
        arg_options.mic_prefix = ("PERFEXPERT_HPCTOOLKIT_MIC_PREFIX");
        OUTPUT_VERBOSE((1, "ENV: mic_prefix=%s", arg_options.mic_prefix));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_MIC_BEFORE")) {
        arg_options.mic_before = ("PERFEXPERT_HPCTOOLKIT_MIC_BEFORE");
        OUTPUT_VERBOSE((1, "ENV: mic_before=%s", arg_options.mic_before));
    }

    if (NULL != getenv("PERFEXPERT_HPCTOOLKIT_MIC_AFTER")) {
        arg_options.mic_after = ("PERFEXPERT_HPCTOOLKIT_MIC_AFTER");
        OUTPUT_VERBOSE((1, "ENV: mic)after=%s", arg_options.mic_after));
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
