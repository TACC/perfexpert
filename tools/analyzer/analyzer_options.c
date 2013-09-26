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
#include "analyzer.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"
#include "install_dirs.h"

/* Structure to handle command line arguments. Try to keep the content of
 * this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"aggregate",        no_argument,       NULL, 'a'},
    {"colorful",         no_argument,       NULL, 'c'},
    {"help",             no_argument,       NULL, 'h'},
    {"inputfile",        required_argument, NULL, 'i'},
    {"lcpifile",         required_argument, NULL, 'l'},
    {"measurement-tool", required_argument, NULL, 'm'},
    {"machine",          required_argument, NULL, 'M'},
    {"outputfile",       required_argument, NULL, 'o'},
    {"sorting-order",    required_argument, NULL, 'O'},
    {"threshold",        required_argument, NULL, 't'},
    {"thread",           required_argument, NULL, 'T'},
    {"verbose",          required_argument, NULL, 'v'},
    {0, 0, 0, 0}
};

/* show_help */
void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: analyzer [-acfhilmMoOTvw]\n\n");
    printf("  -a --aggregate     Show whole-program information (instead of per hotspot)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     appear on output files\n");
    printf("  -h --help          Show this message\n");
    printf("  -i --inputfile     Use argument as input for performance measurements\n");
    printf("  -l --lcpi          Set input file for LCPI metrics\n");
    printf("                     (default: %s/%s)\n", PERFEXPERT_ETCDIR, LCPI_FILE);
    printf("  -m --measurement-tool\n");
    printf("                     Set the tool name used to collect performance measurements\n");
    printf("                     (valid options: hpctoolkit and vtune, default: hpctoolkit)\n");
    printf("  -M --machine       Set input file for hardware characterization\n");
    printf("                     (default: %s/%s)\n", PERFEXPERT_ETCDIR, MACHINE_FILE);
    printf("  -o --outputfile    Set output file for performance analysis (default: STDOUT)\n");
    printf("  -O --sorting-order Define the order in which hotspots should be sorted\n");
    printf("                     (valid values: relevance, performance, and mixed)\n");
    printf("  -t --threshold     Define the relevance (in %% of runtime) of code bottlenecks\n");
    printf("                     PerfExpert should take into consideration (> 0 and <= 1)\n");
    printf("  -T --thread        Show information only for a specific thread ID\n");
    printf("  -v --verbose       Enable verbose mode (default: disabled, levels: 1-10)\n");
}

/* parse_env_vars */
int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL")) {
        globals.verbose = atoi(getenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((1, "ENV: verbose_level=%d", globals.verbose));
    }
    // No variable for -a --aggregate
    if ((NULL != getenv("PERFEXPERT_ANALYZER_COLORFUL")) &&
        (PERFEXPERT_TRUE == atoi(getenv("PERFEXPERT_ANALYZER_COLORFUL")))) {
        globals.colorful = PERFEXPERT_TRUE;
        OUTPUT_VERBOSE((1, "ENV: colorful=YES"));
    }
    // No variable for -h --help
    if (NULL != getenv("PERFEXPERT_ANALYZER_INPUT_FILE")) {
        globals.inputfile = getenv("PERFEXPERT_ANALYZER_INPUT_FILE");
        OUTPUT_VERBOSE((1, "ENV: inputfile=%s", globals.inputfile));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_LCPI_FILE")) {
        globals.lcpifile = getenv("PERFEXPERT_ANALYZER_LCPI_FILE");
        OUTPUT_VERBOSE((1, "ENV: lcpifile=%s", globals.lcpifile));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_TOOL")) {
        globals.tool = getenv("PERFEXPERT_ANALYZER_TOOL");
        OUTPUT_VERBOSE((1, "ENV: tool=%s", globals.tool));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_MACHINE_FILE")) {
        globals.machinefile = getenv("PERFEXPERT_ANALYZER_MACHINE_FILE");
        OUTPUT_VERBOSE((1, "ENV: machinefile=%s", globals.machinefile));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_OUTPUT_FILE")) {
        globals.outputfile = getenv("PERFEXPERT_ANALYZER_OUTPUT_FILE");
        OUTPUT_VERBOSE((1, "ENV: outputfile=%s", globals.outputfile));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_SORTING_ORDER")) {
        globals.order = getenv("PERFEXPERT_ANALYZER_SORTING_ORDER");
        OUTPUT_VERBOSE((1, "ENV: order=%s", globals.order));
    }
    if (NULL != getenv("PERFEXPERT_ANALYZER_THRESHOLD")) {
        globals.threshold = atof(getenv("PERFEXPERT_ANALYZER_THRESHOLD"));
        OUTPUT_VERBOSE((1, "ENV: threshold=%f", globals.threshold));
    }
    // No variable for -T --thread
    if (NULL != getenv("PERFEXPERT_ANALYZER_METRICS_FILE")) {
        globals.outputmetrics = getenv("PERFEXPERT_ANALYZER_METRICS_FILE");
        OUTPUT_VERBOSE((1, "ENV: outputmetrics=%s", globals.outputmetrics));
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
        parameter = getopt_long(argc, argv, "achi:l:m:M:o:O:t:T:v:",
            long_options, &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Aggregated metrics */
            case 'a':
                globals.aggregate = PERFEXPERT_TRUE;
                OUTPUT_VERBOSE((1, "option 'a' set"));
                break;
            /* Activate colorful mode */
            case 'c':
                globals.colorful = PERFEXPERT_TRUE;
                OUTPUT_VERBOSE((1, "option 'c' set"));
                break;
            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((1, "option 'h' set"));
                show_help();
                return PERFEXPERT_ERROR;
            /* Input file */
            case 'i':
                globals.inputfile = optarg;
                OUTPUT_VERBOSE((1, "option 'i' set [%s]", globals.inputfile));
                break;
            /* LCPI file */
            case 'l':
                globals.lcpifile = optarg;
                OUTPUT_VERBOSE((1, "option 'l' set [%s]", globals.lcpifile));
                break;
            /* Measurement tool */
            case 'm':
                globals.tool = optarg;
                OUTPUT_VERBOSE((1, "option 'm' set [%s]", globals.tool));
                break;
            /* Machine file */
            case 'M':
                globals.machinefile = optarg;
                OUTPUT_VERBOSE((1, "option 'M' set [%s]",
                    globals.machinefile));
                break;
            /* Output file */
            case 'o':
                globals.outputfile = optarg;
                OUTPUT_VERBOSE((1, "option 'o' set [%s]", globals.outputfile));
                break;
            /* Sorting order */
            case 'O':
                globals.order = optarg;
                OUTPUT_VERBOSE((1, "option 'O' set [%s]", globals.order));
                break;
            /* Threshold */
            case 't':
                globals.threshold = atof(optarg);
                OUTPUT_VERBOSE((1, "option 't' set [%f]", globals.threshold));
                break;
            /* Thread ID */
            case 'T':
                globals.thread = atoi(optarg);
                OUTPUT_VERBOSE((1, "option 'T' set [%d]", globals.thread));
                break;
            /* Verbose */
            case 'v':
                globals.verbose = atoi(optarg);
                OUTPUT_VERBOSE((1, "option 'v' set [%d]", globals.verbose));
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
    OUTPUT_VERBOSE((7, "   Aggregate?        %s", globals.aggregate ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   Colorful verbose? %s", globals.colorful ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   Input file:       %s", globals.inputfile));
    OUTPUT_VERBOSE((7, "   LCPI file:        %s", globals.lcpifile));
    OUTPUT_VERBOSE((7, "   Measurement tool: %s", globals.tool));
    OUTPUT_VERBOSE((7, "   Machine file:     %s", globals.machinefile));
    OUTPUT_VERBOSE((7, "   Output file:      %s", globals.outputfile));
    OUTPUT_VERBOSE((7, "   Sorting order:    %s", globals.order));
    OUTPUT_VERBOSE((7, "   Threshold:        %f", globals.threshold));
    OUTPUT_VERBOSE((7, "   Thread ID:        %d", globals.thread));
    OUTPUT_VERBOSE((7, "   Verbose level:    %d", globals.verbose));
    OUTPUT_VERBOSE((7, "   Output metrcis:   %s", globals.outputmetrics));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Sanity check: verbose level should be between 1-10 */
    if ((0 > globals.verbose) || (10 < globals.verbose)) {
        OUTPUT(("%s", _ERROR("Error: invalid verbose level")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: threshold is mandatory */
    if ((0 >= globals.threshold) || (1 < globals.threshold)) {
        OUTPUT(("%s", _ERROR("Error: undefined or invalid threshold")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: inputfile is mandatory */
    if (NULL == globals.inputfile) {
        OUTPUT(("%s", _ERROR("Error: undefined input file")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: thread ID must be <= 0, but -1 means NO_THREADS */
    if (-1 > globals.thread) {
        OUTPUT(("%s", _ERROR("Error: invalid thread ID")));
        show_help();
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF

