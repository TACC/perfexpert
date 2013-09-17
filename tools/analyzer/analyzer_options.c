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
#include <getopt.h>

/* PerfExpert headers */
#include "config.h"
#include "analyzer.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"

/* show_help */
void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: analyzer [-m tool] -f file [-achv] [-t thread_id] [-l level]\n\n");
    printf("  -m --measurement   Set the tool name used to collect performance measurements\n");
    printf("                     (valid options: hpctoolkit and vtune, default: hpctoolkit)\n");
    printf("  -f --inputfile     Use 'file' as input for performance measurements\n");
    printf("  -a --aggregate     Show whole-program information (instead of per hotspot)\n");
    printf("  -t --thread        Show information only for a specific thread ID\n");
    printf("  -v --verbose       Enable verbose mode using default verbose level (5)\n");
    printf("  -l --verbose_level Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     appear on output files\n");
    printf("  -h --help          Show this message\n");
}

/* parse_env_vars */
int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL")) {
        if ((0 >= atoi(getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL"))) ||
            (10 < atoi(getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL")))) {
            OUTPUT(("%s (%d)", _ERROR("ENV Error: invalid debug level"),
                atoi(getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL"))));
            show_help();
            return PERFEXPERT_ERROR;
        }
        globals.verbose_level =
            atoi(getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
    }

    if (NULL != getenv("PERFEXPERT_ANALYZER_INPUT_FILE")) {
        globals.inputfile = getenv("PERFEXPERT_ANALYZER_INPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.inputfile));
    }

    if (NULL != getenv("PERFEXPERT_ANALYZER_TOOL")) {
        globals.inputfile = getenv("PERFEXPERT_ANALYZER_TOOL");
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.tool));
    }

    if ((NULL != getenv("PERFEXPERT_ANALYZER_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_ANALYZER_COLORFUL")))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
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
        parameter = getopt_long(argc, argv, "acvhf:l:t:", long_options,
            &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Verbose level */
            case 'l':
                globals.verbose_level = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'l' set"));
                if ((0 >= atoi(optarg)) || (10 < atoi(optarg))) {
                    OUTPUT(("%s (%d)", _ERROR("Error: invalid debug level"),
                        atoi(optarg)));
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Activate verbose mode */
            case 'v':
                if (0 == globals.verbose_level) {
                    globals.verbose_level = 5;
                }
                OUTPUT_VERBOSE((10, "option 'v' set"));
                break;

            /* Activate colorful mode */
            case 'c':
                globals.colorful = PERFEXPERT_TRUE;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Aggregated metrics */
            case 'a':
                globals.aggregate = PERFEXPERT_TRUE;
                OUTPUT_VERBOSE((10, "option 'a' set"));
                break;

            /* Thread */
            case 't':
                globals.thread = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'l' set"));
                if (0 > atoi(optarg)) {
                    OUTPUT(("%s (%d)", _ERROR("Error: invalid thread ID"),
                        atoi(optarg)));
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                return PERFEXPERT_ERROR;

            /* Input file */
            case 'f':
                globals.inputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'f' set [%s]", globals.inputfile));
                break;

            /* Measurement tool */
            case 'm':
                globals.tool = optarg;
                OUTPUT_VERBOSE((10, "option 'm' set [%s]", globals.tool));
                break;

            /* Unknown option */
            case '?':
            default:
                show_help();
                return PERFEXPERT_ERROR;
        }
    }
    OUTPUT_VERBOSE((4, "=== %s", _BLUE("CLI params")));
    OUTPUT_VERBOSE((10, "Summary of selected options:"));
    OUTPUT_VERBOSE((10, "   Verbose level:    %d", globals.verbose_level));
    OUTPUT_VERBOSE((10, "   Aggregate?        %s",
                    globals.aggregate ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Thread ID:        %d", globals.thread));
    OUTPUT_VERBOSE((10, "   Colorful verbose? %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Input file:       %s", globals.inputfile));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Sanity check: inputfile is mandatory */
    if (NULL == globals.inputfile) {
        show_help();
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF

