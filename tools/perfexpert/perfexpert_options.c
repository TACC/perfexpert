/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
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

/* PerfExpert headers */
#include "perfexpert.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_options.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* parse_cli_params */
int parse_cli_params(int argc, char *argv[]) {
    // static struct argp argp = { options, parse_opt, args_doc, doc };
    struct argp argp = { options, parse_options, args_doc, doc };

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("Error: parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, &globals);

    /* Sanity check: verbose level should be between 1-10 */
    if ((0 > globals.verbose) || (10 < globals.verbose)) {
        OUTPUT(("%s", _ERROR("Error: invalid verbose level")));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: threshold is mandatory */
    if ((0 >= globals.threshold) || (1 < globals.threshold)) {
        OUTPUT(("%s", _ERROR("Error: undefined or invalid threshold")));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: NULL program */
    if (NULL == globals.program) {
        OUTPUT(("%s", _ERROR("Error: undefined program")));
        return PERFEXPERT_ERROR;
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
            globals.program, &(globals.program))) {
            OUTPUT(("%s", _ERROR("Error: unable to extract program name")));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((1, "   program only=[%s]", globals.program));

        if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(globals.program,
            &(globals.program_path))) {
            OUTPUT(("%s", _ERROR("Error: unable to extract program path")));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((1, "   program path=[%s]", globals.program_path));

        PERFEXPERT_ALLOC(char, globals.program_full, (strlen(globals.program) +
            strlen(globals.program_path) + 1));
        sprintf(globals.program_full, "%s%s", globals.program_path,
            globals.program);
        OUTPUT_VERBOSE((1, "   program full path=[%s]", globals.program_full));

        if ((NULL == globals.sourcefile) && (NULL == globals.target) &&
            (PERFEXPERT_SUCCESS != perfexpert_util_file_is_exec(
                globals.program_full))) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to find program"),
                globals.program_full));
            return PERFEXPERT_ERROR;
        }
    }

    /* Sanity check: target and sourcefile at the same time */
    if ((NULL != globals.target) && (NULL != globals.sourcefile)) {
        OUTPUT(("%s", _ERROR("Error: target and sourcefile are both defined")));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != globals.knc_prefix) && (NULL == globals.knc)) {
        OUTPUT(("%s", _RED("Warning: option -P selected without option -k")));
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != globals.knc_before) && (NULL == globals.knc)) {
        OUTPUT(("%s", _RED("Warning: option -B selected without option -k")));
    }

    /* Sanity check: MIC options without MIC */
    if ((NULL != globals.knc_after) && (NULL == globals.knc)) {
        OUTPUT(("%s", _RED("Warning: option -A selected without option -k")));
    }

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i = 0;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("program arguments:"));
        while (NULL != globals.program_argv[i]) {
            printf(" [%s]", globals.program_argv[i]);
            i++;
        }
        globals.program_argc = i;
        printf("\n");
    }

    OUTPUT_VERBOSE((7, "%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   Verbose level:       %d", globals.verbose));
    OUTPUT_VERBOSE((7, "   Colorful verbose?    %s", globals.colorful ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   Leave garbage?       %s", globals.leave_garbage ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   Database file:       %s", globals.dbfile));
    OUTPUT_VERBOSE((7, "   Recommendations      %d", globals.rec_count));
    OUTPUT_VERBOSE((7, "   Threshold:           %f", globals.threshold));
    OUTPUT_VERBOSE((7, "   Make target:         %s", globals.target));
    OUTPUT_VERBOSE((7, "   Program source file: %s", globals.sourcefile));
    OUTPUT_VERBOSE((7, "   Program executable:  %s", globals.program));
    OUTPUT_VERBOSE((7, "   Program path:        %s", globals.program_path));
    OUTPUT_VERBOSE((7, "   Program full path:   %s", globals.program_full));
    OUTPUT_VERBOSE((7, "   Program arguments #: %d", globals.program_argc));
    OUTPUT_VERBOSE((7, "   Prefix:              %s", globals.prefix));
    OUTPUT_VERBOSE((7, "   Before each run:     %s", globals.before));
    OUTPUT_VERBOSE((7, "   After each run:      %s", globals.after));
    OUTPUT_VERBOSE((7, "   MIC card:            %s", globals.knc));
    OUTPUT_VERBOSE((7, "   MIC prefix:          %s", globals.knc_prefix));
    OUTPUT_VERBOSE((7, "   MIC before each run: %s", globals.knc_before));
    OUTPUT_VERBOSE((7, "   MIC after each run:  %s", globals.knc_after));
    OUTPUT_VERBOSE((7, "   Sorting order:       %s", globals.order));
    OUTPUT_VERBOSE((7, "   Measurement tool:    %s", globals.tool));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i = 0;
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
        /* Should I run some program after each execution? */
        case 'a':
            globals.after = arg;
            OUTPUT_VERBOSE((1, "option 'a' set [%s]", globals.after));
            break;

        /* Should I run on the KNC some program after each execution? */
        case 'A':
            globals.knc_after = arg;
            OUTPUT_VERBOSE((1, "option 'A' set [%s]", globals.knc_after));
            break;

        /* Should I run some program before each execution? */
        case 'b':
            globals.before = arg;
            OUTPUT_VERBOSE((1, "option 'b' set [%s]", globals.before));
            break;

        /* Should I run on the KNC some program before each execution? */
        case 'B':
            globals.knc_before = arg;
            OUTPUT_VERBOSE((1, "option 'B' set [%s]", globals.knc_before));
            break;

        /* Activate colorful mode */
        case 'c':
            globals.colorful = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'c' set"));
            break;

        /* Which database file? */
        case 'd':
            globals.dbfile = arg;
            OUTPUT_VERBOSE((1, "option 'd' set [%s]", globals.dbfile));
            break;

        /* Leave the garbage there? */
        case 'g':
            globals.leave_garbage = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'g' set"));
            break;

        /* Show help */
        case 'h':
            OUTPUT_VERBOSE((1, "option 'h' set"));
            argp_usage(state);

        /* MIC card */
        case 'k':
            globals.knc = arg;
            OUTPUT_VERBOSE((1, "option 'k' set [%s]", globals.knc));
            break;

        /* Use Makefile? */
        case 'm':
            globals.target = arg;
            OUTPUT_VERBOSE((1, "option 'm' set [%s]", globals.target));
            break;

        /* Sorting order */
        case 'o':
            globals.order = arg;
            OUTPUT_VERBOSE((1, "option 'o' set [%s]", globals.order));
            break;

        /* Should I add a program prefix to the command line? */
        case 'p':
            globals.prefix = arg;
            OUTPUT_VERBOSE((1, "option 'p' set [%s]", globals.prefix));
            break;

        /* Should I add a program prefix to the KNC command line? */
        case 'P':
            globals.knc_prefix = arg;
            OUTPUT_VERBOSE((1, "option 'P' set [%s]", globals.knc_prefix));
            break;

        /* Number of recommendation to output */
        case 'r':
            globals.rec_count = atoi(arg);
            OUTPUT_VERBOSE((1, "option 'r' set [%d]", globals.rec_count));
            break;

        /* What is the source code filename? */
        case 's':
            globals.sourcefile = arg;
            OUTPUT_VERBOSE((1, "option 's' set [%s]", globals.sourcefile));
            break;

        /* Measurement tool */
        case 't':
            globals.tool = arg;
            OUTPUT_VERBOSE((1, "option 't' set [%s]", globals.tool));
            break;

        /* Verbose level */
        case 'v':
            globals.verbose = arg ? atoi(arg) : 5;
            OUTPUT_VERBOSE((1, "option 'v' set [%d]", globals.verbose));
            break;

        /* Arguments: threashold and target program and it's arguments */
        case ARGP_KEY_ARG:
            if (0 == state->arg_num) {
                globals.threshold = atof(arg);
                OUTPUT_VERBOSE((1, "option 'threshold' set [%f] (%s)",
                    globals.threshold, arg));
            }
            if (1 == state->arg_num) {
                globals.program = arg;
                OUTPUT_VERBOSE((1, "option 'target_program' set [%s]",
                    globals.program));
                globals.program_argv = &state->argv[state->next];
                OUTPUT_VERBOSE((1, "option 'program_arguments' set"));
                state->next = state->argc;
            }
            break;

        /* If the arguments are missing */
        case ARGP_KEY_NO_ARGS:
            argp_usage(state);

        /* Too few options */
        case ARGP_KEY_END:
            if (2 > state->arg_num) {
                argp_usage(state);
            }
            break;

        /* Unknown option */
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* parse_env_vars */
static int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_VERBOSE_LEVEL")) {
        globals.verbose = atoi(getenv("PERFEXPERT_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((1, "ENV: verbose_level=%d", globals.verbose));
    }

    if (NULL != getenv("PERFEXPERT_DATABASE_FILE")) {
        globals.dbfile = getenv("PERFEXPERT_DATABASE_FILE");
        OUTPUT_VERBOSE((1, "ENV: dbfile=%s", globals.dbfile));
    }

    if ((NULL != getenv("PERFEXPERT_REC_COUNT")) &&
        (0 <= atoi(getenv("PERFEXPERT_REC_COUNT")))) {
        globals.rec_count = atoi(getenv("PERFEXPERT_REC_COUNT"));
        OUTPUT_VERBOSE((1, "ENV: rec_count=%d", globals.rec_count));
    }

    if ((NULL != getenv("PERFEXPERT_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_COLORFUL")))) {
        globals.colorful = PERFEXPERT_FALSE;
        OUTPUT_VERBOSE((1, "ENV: colorful=YES"));
    }

    if (NULL != getenv("PERFEXPERT_MAKE_TARGET")) {
        globals.target = getenv("PERFEXPERT_MAKE_TARGET");
        OUTPUT_VERBOSE((1, "ENV: target=%s", globals.target));
    }

    if (NULL != getenv("PERFEXPERT_SOURCE_FILE")) {
        globals.sourcefile = ("PERFEXPERT_SOURCE_FILE");
        OUTPUT_VERBOSE((1, "ENV: sourcefile=%s", globals.sourcefile));
    }

    if (NULL != getenv("PERFEXPERT_PREFIX")) {
        globals.prefix = ("PERFEXPERT_PREFIX");
        OUTPUT_VERBOSE((1, "ENV: prefix=%s", globals.prefix));
    }

    if (NULL != getenv("PERFEXPERT_BEFORE")) {
        globals.before = ("PERFEXPERT_BEFORE");
        OUTPUT_VERBOSE((1, "ENV: before=%s", globals.before));
    }

    if (NULL != getenv("PERFEXPERT_AFTER")) {
        globals.after = ("PERFEXPERT_AFTER");
        OUTPUT_VERBOSE((1, "ENV: after=%s", globals.after));
    }

    if (NULL != getenv("PERFEXPERT_KNC_CARD")) {
        globals.knc = ("PERFEXPERT_KNC_CARD");
        OUTPUT_VERBOSE((1, "ENV: knc=%s", globals.knc));
    }

    if (NULL != getenv("PERFEXPERT_KNC_PREFIX")) {
        globals.knc_prefix = ("PERFEXPERT_KNC_PREFIX");
        OUTPUT_VERBOSE((1, "ENV: knc_prefix=%s", globals.knc_prefix));
    }

    if (NULL != getenv("PERFEXPERT_KNC_BEFORE")) {
        globals.knc_before = ("PERFEXPERT_KNC_BEFORE");
        OUTPUT_VERBOSE((1, "ENV: knc_before=%s", globals.knc_before));
    }

    if (NULL != getenv("PERFEXPERT_KNC_AFTER")) {
        globals.knc_after = ("PERFEXPERT_KNC_AFTER");
        OUTPUT_VERBOSE((1, "ENV: knc)after=%s", globals.knc_after));
    }

    if (NULL != getenv("PERFEXPERT_SORTING_ORDER")) {
        globals.order = getenv("PERFEXPERT_SORTING_ORDER");
        OUTPUT_VERBOSE((1, "ENV: order=%s", globals.order));
    }

    if (NULL != getenv("PERFEXPERT_ANALYZER_TOOL")) {
        globals.tool = getenv("PERFEXPERT_ANALYZER_TOOL");
        OUTPUT_VERBOSE((1, "ENV: tool=%s", globals.tool));
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
