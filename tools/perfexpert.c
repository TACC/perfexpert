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
#include "perfexpert_fork.h"
#include "install_dirs.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    char workdir[] = ".perfexpert-temp.XXXXXX";
    char temp_str[2][BUFFER_SIZE];
    int rc = PERFEXPERT_ERROR;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level      = 0,    // int
        .dbfile             = NULL, // char *
        .colorful           = 0,    // int
        .threshold          = 0.0,  // float
        .rec_count          = 3,    // int
        .left_garbage       = 0,    // int
        .target             = NULL, // char *
        .sourcefile         = NULL, // char *
        .program            = NULL, // char *
        .arguments          = 0,    // int
        .arguments_position = 0,    // int
        .before             = NULL, // char *
        .after              = NULL, // char *
        .prefix             = NULL, // char *
        .step               = 1,    // int
        .workdir            = NULL, // char *
        .stepdir            = NULL  // char *
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        goto clean_up;
    }

    /* Create a work directory */
    globals.workdir = mkdtemp(workdir);
    if (NULL == globals.workdir) {
        OUTPUT(("%s", _ERROR("Error: creating working directory")));
        goto clean_up;
    }
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));

    /* If database was not specified, check if the local database is update */
    if (NULL == globals.dbfile) {
        bzero(temp_str[0], BUFFER_SIZE);
        sprintf(temp_str[0], "%s/%s", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
        bzero(temp_str[1], BUFFER_SIZE);
        sprintf(temp_str[1], "%s/.%s", getenv("HOME"), RECOMMENDATION_DB);

        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(temp_str[1])) {
            if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(temp_str[1],
                                                                temp_str[0])) {
                OUTPUT(("%s", _ERROR((char *)"Error: unable to copy file")));
                goto clean_up;
            }
        }
        globals.dbfile = temp_str[1];
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile)) {
            OUTPUT(("%s", _ERROR((char *)"Error: database file not found")));
            goto clean_up;
        }
    }

    /* Iterate until some tool return != PERFEXPERT_SUCCESS */
    while (1) {
        /* Create step working directory */
        globals.stepdir = (char *)malloc(strlen(globals.workdir) + 5);
        if (NULL == globals.stepdir) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            goto clean_up;
        }
        bzero(globals.stepdir, strlen(globals.workdir) + 5);
        sprintf(globals.stepdir, "%s/%d", globals.workdir, globals.step);
        if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.stepdir,
            0755)) {
            OUTPUT(("%s", _ERROR((char *)"Error: cannot create step workdir")));
            goto clean_up;
        }
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("stepdir:"), globals.stepdir));

        /* If necessary, compile the user program */
        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            if (PERFEXPERT_SUCCESS != compile_program()) {
                OUTPUT(("%s", _ERROR("Error: program compilation failed")));
                goto clean_up;
            }
        }

        /* Call HPCToolkit and stuff (former perfexpert_run_exp) */
        if (PERFEXPERT_SUCCESS != measurements()) {
            OUTPUT(("%s", _ERROR("Error: unable to take measurements")));
            goto clean_up;
        }

        /* Call analyzer and stuff (former perfexpert) */
        if (PERFEXPERT_SUCCESS != analysis()) {
            OUTPUT(("%s", _ERROR("Error: unable to run analyzer")));
            goto clean_up;
        }

        /* Call recommender */
        switch (recommendation()) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
                OUTPUT(("%s", _ERROR("Error: unable to run recommender")));
                goto clean_up;

            case PERFEXPERT_NO_REC:
                OUTPUT(("No recommendation found"));
                // TODO: show analysis

                rc = PERFEXPERT_NO_REC;
                goto clean_up;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        /* Call code transformer */
        switch (transformation()) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
                OUTPUT(("%s", _ERROR("Error: unable to run code transformer")));
                goto clean_up;

            case PERFEXPERT_NO_TRANS:
                OUTPUT(("Unable to apply optimizations automatically"));
                // TODO: show analysis and recommendations

                rc = PERFEXPERT_NO_TRANS;
                goto clean_up;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        globals.step++;
    }

    clean_up:
    /* TODO: Should I remove the garbage? */
    if (!globals.left_garbage) {
    }

    /* TODO: Free memory */
    if (NULL != globals.stepdir) {
        free(globals.stepdir);
    }

    return rc;
}

/* show_help */
static void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: perfexpert <threshold> [-gvch] [-l level] [-d database] [-r count]\n");
    printf("                 [-m target|-s sourcefile] [-p prefix] [-a program] [-b program]\n");
    printf("                 <program_executable> [program arguments]\n\n");
    printf("  -d --database      Select the recommendation database file\n");
    printf("                     (default: %s/%s)\n", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
    printf("  -r --recommend     Number of recommendation to show\n");
    printf("  -m --makefile      Use GNU standard 'make' command to compile the code (it\n");
    printf("                     requires the source code available in current directory)\n");
    printf("  -s --source        Specify the source code file (if your source code has more\n");
    printf("                     than one file please use a Makefile and choose '-m' option\n");
    printf("                     it also enables the automatic optimization option '-a')\n");
    printf("  -p --prefix        Add a prefix to the command line (e.g. mpirun) use double\n");
    printf("                     quotes to specify multiple arguments (e.g. -p \"mpirun -n 2\"\n");
    printf("  -b --before        Execute FILE before each run of the application\n");
    printf("  -a --after         Execute FILE after each run of the application\n");
    printf("  -v --verbose       Enable verbose mode using default verbose level (1)\n");
    printf("  -l --verbose_level Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     appear on output files\n");
    printf("  -h --help          Show this message\n\n");
    printf("Use CC, CFLAGS and LDFLAGS to select compiler and compilation/linkage flags\n");
}

/* parse_env_vars */
static int parse_env_vars(void) {
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

    return PERFEXPERT_SUCCESS;
}

/* parse_cli_params */
static int parse_cli_params(int argc, char *argv[]) {
    int parameter;        /** Temporary variable to hold parameter */
    int option_index = 0; /** getopt_long() stores the option index here */

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("Error: parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "a:b:cd:ghl:m:p:r:s:v", long_options,
                                &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Should I run some program after each execution? */
            case 'a':
                globals.after = optarg;
                OUTPUT_VERBOSE((10, "option 'a' set [%s]", globals.after));
                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_exists_and_is_exec(globals.after)) {
                    show_help();
                    return PERFEXPERT_ERROR;
                }
                break;

            /* Should I run some program before each execution? */
            case 'b':
                globals.before = optarg;
                OUTPUT_VERBOSE((10, "option 'b' set [%s]", globals.before));
                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_exists_and_is_exec(globals.before)) {
                    show_help();
                    return PERFEXPERT_ERROR;
                }
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
                globals.left_garbage = 1;
                OUTPUT_VERBOSE((10, "option 'g' set"));
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                exit(PERFEXPERT_SUCCESS);

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

            case 'p':
                globals.prefix = optarg;
                OUTPUT_VERBOSE((10, "option 'p' set [%s]", globals.prefix));
                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_exists_and_is_exec(globals.prefix)) {
                    show_help();
                    return PERFEXPERT_ERROR;
                }
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
            OUTPUT(("%s (%s)", _ERROR("Error: invalid threshold value"),
                    argv[optind]));
            show_help();
            return PERFEXPERT_ERROR;
        }
    }
    optind++;

    if (argc > optind) {
        globals.program = argv[optind];
        OUTPUT_VERBOSE((10, "option 'program_executable' set [%s]",
                        globals.program));
        if ((NULL == globals.sourcefile) && (NULL == globals.target)) {
            if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(
                globals.program)) {
                show_help();
                return PERFEXPERT_ERROR;
            }
        }
    }
    optind++;

    if (argc > optind) {
        globals.arguments = argc - optind;
        OUTPUT_VERBOSE((10, "option 'program_arguments' set [%d]",
                        globals.arguments));
    }

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
    OUTPUT_VERBOSE((10, "   Leave the garbage?   %s",
                    globals.left_garbage ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Database file:       %s", globals.dbfile));
    OUTPUT_VERBOSE((10, "   Recommendations      %d", globals.rec_count));
    OUTPUT_VERBOSE((10, "   Threshold:           %f", globals.threshold));
    OUTPUT_VERBOSE((10, "   Make target:         %s", globals.target));
    OUTPUT_VERBOSE((10, "   Program source file: %s", globals.sourcefile));
    OUTPUT_VERBOSE((10, "   Program executable:  %s", globals.program));
    OUTPUT_VERBOSE((10, "   Program arguments:   %d", globals.arguments));
    OUTPUT_VERBOSE((10, "   Before each run:     %s", globals.before));
    OUTPUT_VERBOSE((10, "   After each run:      %s", globals.after));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Sanity check */
    if ((0.0 >= globals.threshold) || (NULL == globals.program)) {
        show_help();
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* compile_program */
static int compile_program(void) {
    char   temp_str[BUFFER_SIZE];
    char   *argv[PARAM_SIZE];
    int    arg_index = 0;
    int    flags_size = 0;
    char   flags[BUFFER_SIZE];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Compiling the program")));
    OUTPUT(("Compiling [%s]", globals.program));

    /* If the source file was provided generate compilation command line */
    if (NULL != globals.sourcefile) {
        /* Which compiler should I use? */
        argv[arg_index] = getenv("CC");
        if (NULL == argv[arg_index]) {
            argv[arg_index] = DEFAULT_COMPILER;
        }
        arg_index++;

        /* What are the default and user defined compiler flags? */
        flags_size = strlen(DEFAULT_CFLAGS) + 1;
        if (NULL != getenv("CFLAGS")) {
            flags_size += strlen(DEFAULT_CFLAGS) + 1;
        }
        if (NULL != getenv("PERFEXPERT_CFLAGS")) {
            flags_size += strlen(DEFAULT_CFLAGS) + 1;
        }

        bzero(flags, BUFFER_SIZE);
        strcat(flags, DEFAULT_CFLAGS);
        if (NULL != getenv("CFLAGS")) {
            strcat(flags, getenv("CFLAGS"));
            strcat(flags, " ");
        }
        if (NULL != getenv("PERFEXPERT_CFLAGS")) {
            strcat(flags, getenv("PERFEXPERT_CFLAGS"));
        }

        argv[arg_index] = strtok(flags, " ");
        do {
            arg_index++;
        } while (argv[arg_index] = strtok(NULL, " "));

        /* What should be the compiler output and the source code? */
        argv[arg_index] = "-o";
        arg_index++;
        argv[arg_index] = globals.program;
        arg_index++;
        argv[arg_index] = globals.sourcefile;
        arg_index++;
    }

    /* If the user chose a Makefile... */
    // TODO: I should take into consideration the compilation FLAGS here too
    if (NULL != globals.target) {
        argv[arg_index] = "make";
        arg_index++;
        argv[arg_index] = globals.target;
        arg_index++;        
    }

    /* In both cases we should add a NULL */
    argv[arg_index] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < arg_index; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Fill the ninja test structure... */
    test.info   = globals.sourcefile;
    test.input  = NULL;
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/%s", globals.stepdir, "compile.output");
    test.output = temp_str;

    /* fork_and_wait_and_pray */
    return fork_and_wait(&test, argv);
}

/* measurements */
static int measurements(void) {
    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Measurements phase")));
    OUTPUT(("Running [%s]", globals.program));

    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists_and_is_exec(
        globals.program)) {
        return PERFEXPERT_ERROR;
    }
    /* Create the program structure file */
    if (PERFEXPERT_SUCCESS != run_hpcstruct()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcstruct")));
        return PERFEXPERT_ERROR;
    }
    /* Collect measurements */
    if (PERFEXPERT_SUCCESS != run_hpcrun()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcrun")));
        return PERFEXPERT_ERROR;
    }
    /* Sumarize results */
    if (PERFEXPERT_SUCCESS != run_hpcprof()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcprof")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* analysis */
static int analysis(void) {
    char experiment[BUFFER_SIZE];
    char *argv[6];
    char temp_str[3][BUFFER_SIZE];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Analysis phase")));
    OUTPUT(("Analysing measurements"));

    /* First of all, does the file exist? */
    bzero(experiment, BUFFER_SIZE);
    sprintf(experiment, "%s/database/experiment.xml", globals.stepdir);
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(experiment)) {
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/analyzer", PERFEXPERT_BINDIR);
    argv[0] = temp_str[0];
    argv[1] = "--recommend";
    argv[2] = "--opttran";
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%f", globals.threshold);
    argv[3] = temp_str[1];
    argv[4] = experiment;
    argv[5] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/analysis_metrics.txt", globals.stepdir);
    test.output = temp_str[2];
    test.input  = NULL;
    test.info   = experiment;

    /* Run! (to generate analysis generate metrics for recommender) */
    if (PERFEXPERT_SUCCESS != fork_and_wait(&test, argv)) {
        return PERFEXPERT_ERROR;
    }

    /* Work some arguments... */
    argv[1] = argv[3];
    argv[2] = argv[4];
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 2; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/analysis_report.txt", globals.stepdir);
    test.output = temp_str[2];
    test.input  = NULL;
    test.info   = experiment;

    /* Run! (to generate analysis report) */
    if (PERFEXPERT_SUCCESS != fork_and_wait(&test, argv)) {
        return PERFEXPERT_ERROR;
    }

    /* Remove perfexpert.log */
    if (-1 == remove("perfexpert.log")) {
        OUTPUT(("%s", _ERROR("Error: unable to remove perfexpert.log")));
    }

    return PERFEXPERT_SUCCESS;
}

/* recommendation */
static int recommendation(void) {
    char temp_str[6][BUFFER_SIZE];
    char *argv[6];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Recommendations phase")));
    OUTPUT(("Selecting optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/analysis_metrics.txt", globals.stepdir);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_INPUT_FILE", temp_str[0], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.rec_count);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_REC_COUNT", temp_str[1], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE", globals.dbfile, 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_COLORFUL", temp_str[2], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL", "10", 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s/recommender", PERFEXPERT_BINDIR);
    argv[0] = temp_str[3];
    argv[1] = "--automatic";
    argv[2] = globals.stepdir;
    argv[3] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/recommendations_metrics.txt", globals.stepdir);
    argv[4] = temp_str[4];
    argv[5] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line (WTF?) */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    if (0 == globals.verbose_level) {
        bzero(temp_str[5], BUFFER_SIZE);
        sprintf(temp_str[5], "%s/recommender_metrics.output", globals.stepdir);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;

    /* Run! (to generate recommendation metrics for code transformer) */
    if (PERFEXPERT_SUCCESS != fork_and_wait(&test, argv)) {
        return PERFEXPERT_ERROR;
    }

    /* Work some arguments... */
    argv[1] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/recommendations_report.txt", globals.stepdir);
    argv[2] = temp_str[4];
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 2; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    if (0 == globals.verbose_level) {
        bzero(temp_str[5], BUFFER_SIZE);
        sprintf(temp_str[5], "%s/recommender_report.output", globals.stepdir);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;


    /* Run! (to generate recommendation report) */
    if (PERFEXPERT_SUCCESS != fork_and_wait(&test, argv)) {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* transformation */
static int transformation(void) {
    char temp_str[4][BUFFER_SIZE];
    char *argv[6];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Code transformation phase")));
    OUTPUT(("Applying optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/recommendations_metrics.txt", globals.stepdir);
    if (0 != setenv("PERFEXPERT_CT_INPUT_FILE", temp_str[0], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_CT_DATABASE_FILE", globals.dbfile, 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_CT_VERBOSE_LEVEL", "10", 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_CT_COLORFUL", temp_str[1], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/perfexpert_ct", PERFEXPERT_BINDIR);
    argv[0] = temp_str[2];
    argv[1] = "--automatic";
    argv[2] = globals.stepdir;
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line (WTF?) */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 2; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    if (0 == globals.verbose_level) {
        bzero(temp_str[3], BUFFER_SIZE);
        sprintf(temp_str[3], "%s/perfexpert_ct.output", globals.stepdir);
        test.output = temp_str[3];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info  = NULL;

    /* run_and_fork_and_pray */
    return fork_and_wait(&test, argv);
}

/* run_hpcstruct */
static int run_hpcstruct(void) {
    char   temp_str[2][BUFFER_SIZE];
    char   *argv[5];
    test_t test;

    /* Arguments to run hpcstruct */
    argv[0] = HPCSTRUCT;
    argv[1] = "--output";
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[2] = temp_str[0];
    argv[3] = globals.program;
    argv[4] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 3; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%s/hpcstruct.output", globals.stepdir);
    test.output = temp_str[1];
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    return fork_and_wait(&test, argv);
}

/* run_hpcrun */
static int run_hpcrun(void) {
    FILE   *exp_file_FP;
    char   *exp_file;
    int    input_line = 0;
    char   buffer[BUFFER_SIZE];
    char   *temp_str;
    experiment_t *experiment;
    perfexpert_list_t experiments;
    int    rc = PERFEXPERT_SUCCESS;

    /* Open experiment file */
    exp_file = (char *)malloc(strlen(PERFEXPERT_ETCDIR) +
                              strlen(EXPERIMENT_FILE) + 2);
    if (NULL == exp_file) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(exp_file, strlen(PERFEXPERT_ETCDIR) + strlen(EXPERIMENT_FILE) + 2);
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR, EXPERIMENT_FILE);
    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), exp_file));
        free(exp_file);
        return PERFEXPERT_ERROR;
    }

    /* Initiate the list of experiments */
    perfexpert_list_construct((perfexpert_list_t *)&experiments);

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, exp_file_FP)) {
        int temp;

        /* Ignore comments */
        input_line++;
        if (0 == strncmp("#", buffer, 1)) {
            continue;
        }

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* Create a list item for this code experiment */
            experiment = (experiment_t *)malloc(sizeof(experiment_t));
            if (NULL == experiment) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }

            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)experiment);
            perfexpert_list_append(&experiments,
                                   (perfexpert_list_item_t *)experiment);

            // TODO: add PREFIX to argv

            /* Arguments to run hpcrun */
            experiment->argc = 3;
            experiment->argv[0] = HPCRUN;
            experiment->argv[1] = "--output";
            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/measurements", globals.stepdir);
            experiment->argv[2] = temp_str;

            /* Move to next line */
            continue;
        }

        /* Remove the \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Read event */
        experiment->argv[experiment->argc] = "--event";
        experiment->argc++;

        temp_str = (char *)malloc(BUFFER_SIZE);
        if (NULL == temp_str) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            rc = PERFEXPERT_ERROR;
            goto CLEANUP;
        }
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s", buffer);
        experiment->argv[experiment->argc] = temp_str;
        experiment->argc++;

        /* Move to next line */
        continue;
    }

    /* Close file and free memory */
    fclose(exp_file_FP);
    free(exp_file);

    /* For each experiment... */
    input_line = 1;
    OUTPUT_VERBOSE((5, "   %s", _YELLOW("running experiments")));
    experiment = (experiment_t *)perfexpert_list_get_first(&experiments);
    while ((perfexpert_list_item_t *)experiment != &(experiments.sentinel)) {

        /* Ok, now we just have to add the program and its arguments */
        experiment->argv[experiment->argc] = globals.program;
        experiment->argc++;
        experiment->argv[experiment->argc] = NULL;

        // TODO: add the program arguments to argv!
        // TODO: run the BEFORE and AFTER programs

        /* The super-ninja test sctructure */
        temp_str = (char *)malloc(BUFFER_SIZE);
        if (NULL == temp_str) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            rc = PERFEXPERT_ERROR;
            goto CLEANUP;
        }
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s/hpcrun.%d.output", globals.stepdir, input_line);
        experiment->test.output = temp_str;
        experiment->test.input  = NULL;
        experiment->test.info   = globals.program;

        /* Not using OUTPUT_VERBOSE because I want only one line */
        if (8 <= globals.verbose_level) {
            int i;
            printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
            for (i = 0; i < experiment->argc; i++) {
                printf(" %s", experiment->argv[i]);
            }
            printf("\n");
        }

        /* Test patterns */
        switch (fork_and_wait(&(experiment->test), (char **)experiment->argv)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
                break;

            case PERFEXPERT_SUCCESS:
                 OUTPUT_VERBOSE((7, "   [ %s  ]", _BOLDGREEN("OK")));
                break;

            default:
                break;
        }

        /* Move to the next experiment */
        input_line++;
        experiment = (experiment_t *)perfexpert_list_get_next(experiment);
    }

    CLEANUP:
    /* Free memory */
    // TODO: free list of experiments, nd for each experiment argv and test

    return rc;
}

/* run_hpcprof */
static int run_hpcprof(void) {
    char   temp_str[4][BUFFER_SIZE];
    char   *argv[9];
    test_t test;

    /* Arguments to run hpcprof */
    argv[0] = HPCPROF;
    argv[1] = "--force-metric";
    argv[2] = "--metric=thread";
    argv[3] = "--struct";
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[4] = temp_str[0];
    argv[5] = "--output";
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%s/database", globals.stepdir);
    argv[6] = temp_str[1];
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/measurements", globals.stepdir);
    argv[7] = temp_str[2];
    argv[8] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 7; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s/hpcprof.output", globals.stepdir);
    test.output = temp_str[3];
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    return fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
