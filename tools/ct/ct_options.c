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
#include "ct.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "install_dirs.h"

/* show_help */
void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: perfexpert_ct -f file [-o file] [-vch] [-l level] [-a workdir] [-p pid]");
    printf("                     [-d database]\n\n");
    printf("  -f --inputfile     Use 'file' as input for patterns\n");
    printf("  -o --outputfile    Use 'file' as output (default stdout)\n");
    printf("  -a --automatic     Use automatic performance optimization and create files\n");
    printf("                     into 'dir' directory (default: off).\n");
    printf("  -d --database      Select the recommendation database file\n");
    printf("                     (default: %s/%s)\n", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    printf("  -p --pid           Use 'pid' to log on consecutive calls to recommender\n");
    printf("  -v --verbose       Enable verbose mode using default verbose level (5)\n");
    printf("  -l --verbose_level Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     appear on output files\n");
    printf("  -h --help          Show this message\n");
}

/* parse_env_vars */
int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_CT_VERBOSE_LEVEL")) {
        if ((0 >= atoi(getenv("PERFEXPERT_CT_VERBOSE_LEVEL"))) ||
            (10 < atoi(getenv("PERFEXPERT_CT_VERBOSE_LEVEL")))) {
            OUTPUT(("%s (%d)", _ERROR("ENV Error: invalid debug level"),
                atoi(getenv("PERFEXPERT_CT_VERBOSE_LEVEL"))));
            show_help();
            return PERFEXPERT_ERROR;
        }
        globals.verbose_level = atoi(getenv("PERFEXPERT_CT_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
    }

    if (NULL != getenv("PERFEXPERT_CT_INPUT_FILE")) {
        globals.inputfile = getenv("PERFEXPERT_CT_INPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.inputfile));
    }

    if (NULL != getenv("PERFEXPERT_CT_OUTPUT_FILE")) {
        globals.outputfile = getenv("PERFEXPERT_CT_OUTPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: outputfile=%s", globals.outputfile));
    }

    if (NULL != getenv("PERFEXPERT_CT_DATABASE_FILE")) {
        globals.dbfile =  getenv("PERFEXPERT_CT_DATABASE_FILE");
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }

    if (NULL != getenv("PERFEXPERT_CT_WORKDIR")) {
        globals.workdir = getenv("PERFEXPERT_CT_WORKDIR");
        OUTPUT_VERBOSE((5, "ENV: workdir=%s", globals.workdir));
    }

    if ((NULL != getenv("PERFEXPERT_CT_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_CT_COLORFUL")))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
    }

    if (NULL != getenv("PERFEXPERT_CT_PID")) {
        globals.pid = atoi(getenv("PERFEXPERT_CT_PID"));
        OUTPUT_VERBOSE((5, "ENV: pid=%d", globals.pid));
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
        parameter = getopt_long(argc, argv, "a:cvhf:l:o:p:", long_options,
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

            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OUTPUT_VERBOSE((10, "option 'd' set [%s]", globals.dbfile));
                break;

            /* Specify PID (for LOG) */
            case 'p':
                globals.pid = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'p' set [%d]", globals.pid));
                break;

            /* Activate colorful mode */
            case 'c':
                globals.colorful = 1;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                return PERFEXPERT_ERROR;

            /* Use input file? */
            case 'f':
                globals.inputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'f' set [%s]", globals.inputfile));
                break;

            /* Use output file? */
            case 'o':
                globals.outputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'o' set [%s]", globals.outputfile));
                break;

            /* Use automatic optimization? */
            case 'a':
                globals.workdir = optarg;
                OUTPUT_VERBOSE((10, "option 'a' set [%s]", globals.workdir));
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
    OUTPUT_VERBOSE((10, "   Colorful verbose? %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Input file:       %s", globals.inputfile));
    OUTPUT_VERBOSE((10, "   Output file:      %s", globals.outputfile));
    OUTPUT_VERBOSE((10, "   PID:              %d", globals.pid));
    OUTPUT_VERBOSE((10, "   Work directory:   %s", globals.workdir));
    OUTPUT_VERBOSE((10, "   Database file:    %s", globals.dbfile));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
