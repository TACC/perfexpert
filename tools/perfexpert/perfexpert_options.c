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
#include <getopt.h>

/* PerfExpert headers */
#include "config.h"
#include "perfexpert.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_constants.h"
#include "install_dirs.h"

/* show_help */
void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: perfexpert <threshold> [-m target|-s sourcefile] [-r count] [-d database]\n");
    printf("                  [-p prefix] [-b filename] [-a filename] [-l level] [-gvch]\n");
    printf("                  [-k card [-P prefix] [-B filename] [-A filename] ]\n");
    printf("                  <program_executable> [program_arguments]\n\n");
    printf("  <threshold>        Define the relevance (in %% of runtime) of code fragments\n");
    printf("                     PerfExpert should take into consideration (> 0 and <= 1)\n");
    printf("  -m --makefile      Use GNU standard 'make' command to compile the code (it\n");
    printf("                     requires the source code available in current directory)\n");
    printf("  -s --source        Specify the source code file (if your source code has more\n");
    printf("                     than one file please use a Makefile and choose '-m' option\n");
    printf("                     it also enables the automatic optimization option '-a')\n");
    printf("  -r --recommend     Number of recommendations ('count') PerfExpert should show\n");
    printf("  -d --database      Select the recommendation database file\n");
    printf("                     (default: %s/%s)\n", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    printf("  -p --prefix        Add a prefix to the command line (e.g. mpirun). Use double\n");
    printf("                     quotes to specify arguments with spaces within (e.g.\n");
    printf("                     -p \"mpirun -n 2\"). Use a semicolon (';') to run multiple\n");
    printf("                     commands in the same command line\n");
    printf("  -b --before        Execute 'filename' before each run of the application\n");
    printf("  -a --after         Execute 'filename' after each run of the application\n");
    printf("  -k --knc           Tell PerfExpert to run the experiments on the KNC 'card'\n");
    printf("  -P --prefix-knc    Add a prefix to the command line (e.g. mpirun). Use double\n");
    printf("                     quotes to specify arguments with spaces within (e.g.\n");
    printf("                     -p \"mpirun -n 2\"). Use a semicolon (';') to run multiple\n");
    printf("                     commands in the same command line\n");
    printf("  -B --knc-before    Execute 'filename' before each run of the application on\n");
    printf("                     the KNC card.\n");
    printf("  -A --knc-after     Execute 'filename' after each run of the application on\n");
    printf("                     the KNC card.\n");
    printf("  -g --clean-garbage Remove temporary files after run\n");
    printf("  -v --verbose       Enable verbose mode using default verbose level (1)\n");
    printf("  -l --verbose-level Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     appear on output files\n");
    printf("  -h --help          Show this message\n\n");
    printf("Use CC, CFLAGS and LDFLAGS to select compiler and compilation/linkage flags\n");
}

/* parse_env_vars */
int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_VERBOSE_LEVEL")) {
        if ((0 >= atoi(getenv("PERFEXPERT_VERBOSE_LEVEL"))) ||
            (10 < atoi(getenv("PERFEXPERT_VERBOSE_LEVEL")))) {
            OUTPUT(("%s (%d)", _ERROR("ENV Error: invalid debug level"),
                    atoi(getenv("PERFEXPERT_VERBOSE_LEVEL"))));
            show_help();
            return PERFEXPERT_ERROR;
        }
        globals.verbose_level = atoi(getenv("PERFEXPERT_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
    }

    if (NULL != getenv("PERFEXPERT_DATABASE_FILE")) {
        globals.dbfile = getenv("PERFEXPERT_DATABASE_FILE");
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }

    if ((NULL != getenv("PERFEXPERT_REC_COUNT")) &&
        (0 <= atoi(getenv("PERFEXPERT_REC_COUNT")))) {
        globals.rec_count = atoi(getenv("PERFEXPERT_REC_COUNT"));
        OUTPUT_VERBOSE((5, "ENV: rec_count=%d", globals.rec_count));
    }

    if ((NULL != getenv("PERFEXPERT_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_COLORFUL")))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
    }

    if (NULL != getenv("PERFEXPERT_MAKE_TARGET")) {
        globals.target = getenv("PERFEXPERT_MAKE_TARGET");
        OUTPUT_VERBOSE((5, "ENV: target=%s", globals.target));
    }

    if (NULL != getenv("PERFEXPERT_SOURCE_FILE")) {
        globals.sourcefile = ("PERFEXPERT_SOURCE_FILE");
        OUTPUT_VERBOSE((5, "ENV: sourcefile=%s", globals.sourcefile));
    }

    if (NULL != getenv("PERFEXPERT_PREFIX")) {
        globals.prefix = ("PERFEXPERT_PREFIX");
        OUTPUT_VERBOSE((5, "ENV: prefix=%s", globals.prefix));
    }

    if (NULL != getenv("PERFEXPERT_BEFORE")) {
        globals.before = ("PERFEXPERT_BEFORE");
        OUTPUT_VERBOSE((5, "ENV: before=%s", globals.before));
    }

    if (NULL != getenv("PERFEXPERT_AFTER")) {
        globals.after = ("PERFEXPERT_AFTER");
        OUTPUT_VERBOSE((5, "ENV: after=%s", globals.after));
    }

    if (NULL != getenv("PERFEXPERT_KNC_CARD")) {
        globals.knc = ("PERFEXPERT_KNC_CARD");
        OUTPUT_VERBOSE((5, "ENV: knc=%s", globals.knc));
    }

    if (NULL != getenv("PERFEXPERT_KNC_PREFIX")) {
        globals.knc_prefix = ("PERFEXPERT_KNC_PREFIX");
        OUTPUT_VERBOSE((5, "ENV: knc_prefix=%s", globals.knc_prefix));
    }

    if (NULL != getenv("PERFEXPERT_KNC_BEFORE")) {
        globals.knc_before = ("PERFEXPERT_KNC_BEFORE");
        OUTPUT_VERBOSE((5, "ENV: knc_before=%s", globals.knc_before));
    }

    if (NULL != getenv("PERFEXPERT_KNC_AFTER")) {
        globals.knc_after = ("PERFEXPERT_KNC_AFTER");
        OUTPUT_VERBOSE((5, "ENV: knc)after=%s", globals.knc_after));
    }

    return PERFEXPERT_SUCCESS;
}

/* parse_cli_params */
int parse_cli_params(int argc, char *argv[]) {
    int parameter;        /** Temporary variable to hold parameter */
    int option_index = 0; /** getopt_long() stores the option index here */

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("Error: parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "a:A:b:B:cd:ghk:l:m:p:P:r:s:v",
            long_options, &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Should I run some program after each execution? */
            case 'a':
                globals.after = optarg;
                OUTPUT_VERBOSE((10, "option 'a' set [%s]", globals.after));
                break;

            /* Should I run on the KNC some program after each execution? */
            case 'A':
                globals.knc_after = optarg;
                OUTPUT_VERBOSE((10, "option 'A' set [%s]", globals.knc_after));
                break;

            /* Should I run some program before each execution? */
            case 'b':
                globals.before = optarg;
                OUTPUT_VERBOSE((10, "option 'b' set [%s]", globals.before));
                break;

            /* Should I run on the KNC some program before each execution? */
            case 'B':
                globals.knc_before = optarg;
                OUTPUT_VERBOSE((10, "option 'B' set [%s]", globals.knc_before));
                break;

            /* Activate colorful mode */
            case 'c':
                globals.colorful = 1;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OUTPUT_VERBOSE((10, "option 'd' set [%s]", globals.dbfile));
                break;

            /* Leave the garbage there? */
            case 'g':
                globals.clean_garbage = 1;
                OUTPUT_VERBOSE((10, "option 'g' set"));
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                exit(PERFEXPERT_SUCCESS);

            /* MIC card */
            case 'k':
                globals.knc = optarg;
                OUTPUT_VERBOSE((10, "option 'k' set [%s]", globals.knc));
                break;

            /* Verbose level */
            case 'l':
                globals.verbose_level = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'l' set [%s]", optarg));
                if ((0 >= atoi(optarg)) || (10 < atoi(optarg))) {
                    OUTPUT(("%s (%d)", _ERROR("Error: invalid debug level"),
                            atoi(optarg)));
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Use Makefile? */
            case 'm':
                globals.target = optarg;
                OUTPUT_VERBOSE((10, "option 'm' set"));
                if (NULL != globals.sourcefile) {
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Should I add a program prefix to the command line? */
            case 'p':
                globals.prefix = optarg;
                OUTPUT_VERBOSE((10, "option 'p' set [%s]", globals.prefix));
                break;

            /* Should I add a program prefix to the KNC command line? */
            case 'P':
                globals.knc_prefix = optarg;
                OUTPUT_VERBOSE((10, "option 'P' set [%s]", globals.knc_prefix));
                break;

            /* Number of recommendation to output */
            case 'r':
                globals.rec_count = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'r' set [%d]", globals.rec_count));
                break;

            /* What is the source code filename? */
            case 's':
                globals.sourcefile = optarg;
                OUTPUT_VERBOSE((10, "option 's' set [%s]", globals.sourcefile));
                if (NULL != globals.target) {
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Activate verbose mode */
            case 'v':
                if (0 == globals.verbose_level) {
                    globals.verbose_level = 1;
                }
                OUTPUT_VERBOSE((10, "option 'v' set"));
                break;

            /* Unknown option */
            case '?':
            default:
                show_help();
                return PERFEXPERT_ERROR;
        }
    }

    if (argc > optind) {
        globals.threshold = atof(argv[optind]);
        OUTPUT_VERBOSE((10, "option 'threshold' set [%f]", globals.threshold));
        if ((0 >= globals.threshold) || (1 < globals.threshold)) {
            OUTPUT(("%s", _ERROR("Error: invalid threshold value")));
            show_help();
            return PERFEXPERT_ERROR;
        }
    }
    optind++;

    if (argc > optind) {
        OUTPUT_VERBOSE((10, "option 'program_executable' set [%s]",
            argv[optind]));
        if ((NULL == globals.sourcefile) && (NULL == globals.target)) {
            if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists_and_is_exec(
                argv[optind])) {
                OUTPUT(("%s (%s)", _ERROR("Error: unable to find program"),
                    argv[optind]));
                return PERFEXPERT_ERROR;
            }
            if (PERFEXPERT_SUCCESS != perfexpert_util_program_only(argv[optind],
                &(globals.program))) {
                OUTPUT(("%s", _ERROR("Error: unable to find program")));
                return PERFEXPERT_ERROR;
            }
            if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(argv[optind],
                &(globals.program_path))) {
                OUTPUT(("%s", _ERROR("Error: unable to find program")));
                return PERFEXPERT_ERROR;
            }
            globals.program_full = (char *)malloc(strlen(globals.program) +
                strlen(globals.program_path) + 1);
            if (NULL == globals.program_full) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(globals.program_full, strlen(globals.program) +
                strlen(globals.program_path) + 1);
            sprintf(globals.program_full, "%s%s", globals.program_path,
                globals.program);
        }
    }
    optind++;

    globals.prog_arg_pos = optind;
    OUTPUT_VERBOSE((10, "option 'program_arguments' set [%d]",
        argc - globals.prog_arg_pos));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("program arguments:"));
        while ((argc - 1) >= optind) {
            printf("[%s]", argv[optind++]);
        }
        printf("\n");
    }

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("CLI params")));
    OUTPUT_VERBOSE((10, "Summary of selected options:"));
    OUTPUT_VERBOSE((10, "   Verbose level:       %d", globals.verbose_level));
    OUTPUT_VERBOSE((10, "   Colorful verbose?    %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Clean the garbage?   %s",
                    globals.clean_garbage ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Database file:       %s", globals.dbfile));
    OUTPUT_VERBOSE((10, "   Recommendations      %d", globals.rec_count));
    OUTPUT_VERBOSE((10, "   Threshold:           %f", globals.threshold));
    OUTPUT_VERBOSE((10, "   Make target:         %s", globals.target));
    OUTPUT_VERBOSE((10, "   Program source file: %s", globals.sourcefile));
    OUTPUT_VERBOSE((10, "   Program executable:  %s", globals.program));
    OUTPUT_VERBOSE((10, "   Program path:        %s", globals.program_path));
    OUTPUT_VERBOSE((10, "   Program full path:   %s", globals.program_full));
    OUTPUT_VERBOSE((10, "   Program arguments:   %d",
                    argc - globals.prog_arg_pos));
    OUTPUT_VERBOSE((10, "   Prefix:              %s", globals.prefix));
    OUTPUT_VERBOSE((10, "   Before each run:     %s", globals.before));
    OUTPUT_VERBOSE((10, "   After each run:      %s", globals.after));
    OUTPUT_VERBOSE((10, "   MIC card:            %s", globals.knc));
    OUTPUT_VERBOSE((10, "   MIC prefix:          %s", globals.knc_prefix));
    OUTPUT_VERBOSE((10, "   MIC before each run: %s", globals.knc_before));
    OUTPUT_VERBOSE((10, "   MIC after each run:  %s", globals.knc_after));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Sanity check: threshold program */
    if (0 == globals.threshold) {
        OUTPUT(("%s", _ERROR("Error: undefined threshold")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: NULL program */
    if (NULL == globals.program) {
        OUTPUT(("%s", _ERROR("Error: undefined program")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: target and sourcefile at the same time */
    if ((NULL != globals.target) && (NULL != globals.sourcefile)) {
        OUTPUT(("%s", _ERROR("Error: target and sourcefile are both defined")));
        show_help();
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

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
