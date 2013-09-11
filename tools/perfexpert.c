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
#include <sys/types.h>
#include <sys/stat.h>

/* PerfExpert headers */
#include "config.h"
#include "perfexpert.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_fork.h"
#include "perfexpert_database.h"
#include "perfexpert_log.h"
#include "perfexpert_constants.h"
#include "install_dirs.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    char workdir[] = ".perfexpert-temp.XXXXXX";
    char temp_str[BUFFER_SIZE];
    char *temp_str2;
    int rc = PERFEXPERT_ERROR;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level = 0,              // int
        .dbfile        = NULL,           // char *
        .colorful      = 0,              // int
        .threshold     = 0.0,            // float
        .rec_count     = 3,              // int
        .clean_garbage = 0,              // int
        .pid           = (long)getpid(), // long int
        .target        = NULL,           // char *
        .sourcefile    = NULL,           // char *
        .program       = NULL,           // char *
        .program_path  = NULL,           // char *
        .program_full  = NULL,           // char *
        .prog_arg_pos  = 0,              // int
        .main_argc     = argc,           // int
        .main_argv     = argv,           // char **
        .step          = 1,              // int
        .workdir       = NULL,           // char *
        .stepdir       = NULL,           // char *
        .prefix        = NULL,           // char *
        .before        = NULL,           // char *
        .after         = NULL,           // char *
        .knc           = NULL,           // char *
        .knc_prefix    = NULL,           // char *
        .knc_before    = NULL,           // char *
        .knc_after     = NULL            // char *
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        goto clean_up;
    }

    /* Create a work directory */
    bzero(temp_str, BUFFER_SIZE);
    temp_str2 = mkdtemp(workdir);
    if (NULL == temp_str2) {
        OUTPUT(("%s", _ERROR("Error: creating working directory")));
        goto clean_up;
    }
    globals.workdir = (char *)malloc(strlen(getcwd(NULL, 0)) + strlen(temp_str2)
        + 1);
    if (NULL == globals.workdir) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(globals.workdir, strlen(getcwd(NULL, 0)) + strlen(temp_str2) + 1);
    sprintf(globals.workdir, "%s/%s", getcwd(NULL, 0), temp_str2);
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));

    /* If database was not specified, check if there is any local database and
     * if this database is update
     */
    if (NULL == globals.dbfile) {
        if (PERFEXPERT_SUCCESS !=
            perfexpert_database_update(&(globals.dbfile))) {
            OUTPUT(("%s", _ERROR((char *)"Error: unable to copy database")));
            goto clean_up;
        }        
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile)) {
            OUTPUT(("%s", _ERROR((char *)"Error: database file not found")));
            goto clean_up;
        }
    }
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));

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
                if (NULL != globals.knc) {
                    OUTPUT(("Are you adding the flags to to compile for MIC?"));                    
                }
                goto clean_up;
            }
        }

        /* Call HPCToolkit and stuff (former perfexpert_run_exp) */
        if (PERFEXPERT_SUCCESS != measurements()) {
            OUTPUT(("%s", _ERROR("Error: unable to take measurements")));
            goto clean_up;
        }

        /* Call analyzer and stuff (former perfexpert) */
        switch (analysis()) {
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_ERROR:
                OUTPUT(("%s", _ERROR("Error: unable to run analyzer")));
                goto clean_up;

            case PERFEXPERT_NO_DATA:
                OUTPUT(("Unable to analyze your application"));

                /* Print analysis report */
                bzero(temp_str, BUFFER_SIZE);
                sprintf(temp_str, "%s/%s.txt", globals.stepdir,
                        ANALYZER_REPORT);

                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_print(temp_str)) {
                    OUTPUT(("%s",
                        _ERROR("Error: unable to show analysis report")));
                }
                rc = PERFEXPERT_NO_DATA;
                goto clean_up;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        /* Call recommender */
        switch (recommendation()) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
                OUTPUT(("%s", _ERROR("Error: unable to run recommender")));
                goto clean_up;

            case PERFEXPERT_NO_REC:
                OUTPUT(("No recommendation found, printing analysys report"));
                rc = PERFEXPERT_NO_REC;
                goto report;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            #if HAVE_ROSE == 1
            /* Call code transformer */
            switch (transformation()) {
                case PERFEXPERT_ERROR:
                case PERFEXPERT_FAILURE:
                    OUTPUT(("%s",
                            _ERROR("Error: unable to run code transformer")));
                    goto clean_up;

                case PERFEXPERT_NO_TRANS:
                    OUTPUT(("Unable to apply optimizations automatically"));
                    rc = PERFEXPERT_NO_TRANS;
                    goto report;

                case PERFEXPERT_SUCCESS:
                    rc = PERFEXPERT_SUCCESS;
            }
            #endif
        } else {
            rc = PERFEXPERT_SUCCESS;
            goto report;
        }
        OUTPUT(("Starting another optimization round..."));
        globals.step++;
    }

    /* Print analysis report */
    report:
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/%s.txt", globals.stepdir, ANALYZER_REPORT);

    if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(temp_str)) {
        OUTPUT(("%s", _ERROR("Error: unable to show analysis report")));
    }

    /* Print recommendations */
    if ((0 < globals.rec_count) && (PERFEXPERT_NO_REC != rc)) {
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s/%s.txt", globals.stepdir, RECOMMENDER_REPORT);

        if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(temp_str)) {
            OUTPUT(("%s", _ERROR("Error: unable to show recommendations")));
        }
    }

    clean_up:
    /* TODO: Should I remove the garbage? */
    if (!globals.clean_garbage) {
    }

    /* TODO: Free memory */
    if (NULL != globals.stepdir) {
        free(globals.stepdir);
    }
    /* Remove perfexpert.log */
    remove("perfexpert.log");

    return rc;
}

/* show_help */
static void show_help(void) {
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
    printf("                     (default: %s/%s)\n", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
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
                show_help();
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

/* compile_program */
static int compile_program(void) {
    char   temp_str[BUFFER_SIZE];
    char   *argv[PARAM_SIZE];
    int    arg_index = 0;
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
    if (NULL != globals.target) {
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./Makefile")) {
            OUTPUT(("%s", _ERROR("Error: Makefile file not found")));
            return PERFEXPERT_ERROR;                    
        }

        argv[arg_index] = "make";
        arg_index++;
        argv[arg_index] = globals.target;
        arg_index++;

        if (NULL != getenv("CFLAGS")) {
            strcat(flags, getenv("CFLAGS"));
            strcat(flags, " ");
        }
        if (NULL != getenv("PERFEXPERT_CFLAGS")) {
            strcat(flags, getenv("PERFEXPERT_CFLAGS"));
        }
        setenv("CFLAGS", flags, 1);
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
    if (NULL == globals.knc) {
        if (PERFEXPERT_SUCCESS != run_hpcrun()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun")));
            return PERFEXPERT_ERROR;
        }
    } else {
        if (PERFEXPERT_SUCCESS != run_hpcrun_knc()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun on KNC")));
            OUTPUT(("Are you adding the flags to compile for MIC?"));                    
            return PERFEXPERT_ERROR;
        }
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
    sprintf(temp_str[0], "%s", ANALYZER_PROGRAM);
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
    sprintf(temp_str[2], "%s/%s.txt", globals.stepdir, ANALYZER_METRICS);
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
    sprintf(temp_str[2], "%s/%s.txt", globals.stepdir, ANALYZER_REPORT);
    test.output = temp_str[2];
    test.input  = NULL;
    test.info   = experiment;

    /* Run! (to generate analysis report) */
    return fork_and_wait(&test, argv);
}

/* recommendation */
static int recommendation(void) {
    char temp_str[8][BUFFER_SIZE];
    char *argv[6];
    test_t test;
    int rc;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Recommendations phase")));
    OUTPUT(("Selecting optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.txt", globals.stepdir, ANALYZER_METRICS);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_INPUT_FILE", temp_str[0], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.rec_count);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_REC_COUNT", temp_str[1], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE", globals.dbfile, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_COLORFUL", temp_str[2], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[7], BUFFER_SIZE);
    sprintf(temp_str[7], "%d", globals.verbose_level);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL", temp_str[7], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[6], BUFFER_SIZE);
    sprintf(temp_str[6], "%d", (int)getpid());
    if (0 != setenv("PERFEXPERT_RECOMMENDER_PID", temp_str[6], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s", RECOMMENDER_PROGRAM);
    argv[0] = temp_str[3];
    argv[1] = "--automatic";
    argv[2] = globals.stepdir;
    argv[3] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/%s.txt", globals.stepdir, RECOMMENDER_METRICS);
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
        sprintf(temp_str[5], "%s/%s.output", globals.stepdir,
                RECOMMENDER_METRICS);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;

    /* Run! (to generate recommendation metrics for code transformer) */
    if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
        rc = fork_and_wait(&test, argv);
        if (PERFEXPERT_ERROR == rc) {
            return PERFEXPERT_ERROR;
        }
    }

    /* Work some arguments... */
    argv[1] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/%s.txt", globals.stepdir, RECOMMENDER_REPORT);
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
        sprintf(temp_str[5], "%s/%s.output", globals.stepdir,
                RECOMMENDER_REPORT);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;


    /* Run! (to generate recommendation report) */
    rc = fork_and_wait(&test, argv);
    if (PERFEXPERT_ERROR == rc) {
        return PERFEXPERT_ERROR;
    }

    return rc;
}

/* transformation */
static int transformation(void) {
    char temp_str[6][BUFFER_SIZE];
    char *argv[6];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Code transformation phase")));
    OUTPUT(("Applying optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.txt", globals.stepdir, RECOMMENDER_METRICS);
    if (0 != setenv("PERFEXPERT_CT_INPUT_FILE", temp_str[0], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_CT_DATABASE_FILE", globals.dbfile, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[5], BUFFER_SIZE);
    sprintf(temp_str[5], "%d", globals.verbose_level);
    if (0 != setenv("PERFEXPERT_CT_VERBOSE_LEVEL", temp_str[5], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_CT_COLORFUL", temp_str[1], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%d", (int)getpid());
    if (0 != setenv("PERFEXPERT_CT_PID", temp_str[4], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s", CT_PROGRAM);
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
        sprintf(temp_str[3], "%s/%s.output", globals.stepdir, CT_PROGRAM);
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
    argv[3] = globals.program_full;
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
    char   *argv[2];
    test_t test;

    /* Open experiment file (it is a list of arguments which I use to run) */
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
            experiment->argc = 0;

            /* Add PREFIX to argv */
            if (NULL != globals.prefix) {
                temp_str = (char *)malloc(strlen(globals.prefix) + 1);
                if (NULL == temp_str) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    rc = PERFEXPERT_ERROR;
                    goto CLEANUP;
                }
                bzero(temp_str, strlen(globals.prefix) + 1);
                sprintf(temp_str, "%s", globals.prefix);

                experiment->argv[experiment->argc] = strtok(temp_str, " ");
                do {
                    experiment->argc++;
                } while (experiment->argv[experiment->argc] = strtok(NULL,
                    " "));
            }

            /* Arguments to run hpcrun */
            experiment->argv[experiment->argc] = HPCRUN;
            experiment->argc++;
            experiment->argv[experiment->argc] = "--output";
            experiment->argc++;
            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/measurements", globals.stepdir);
            experiment->argv[experiment->argc] = temp_str;
            experiment->argc++;

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
        int count = 0;

        /* Run the BEFORE program */
        if (NULL != globals.before) {
            argv[0] = globals.before;
            argv[1]; NULL;

            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/before.%d.output", globals.stepdir,
                    input_line);
            experiment->test.output = temp_str;
            test.output = temp_str;
            test.input  = NULL;
            test.info   = globals.before;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _BOLDRED("error running"),
                        globals.before));
            }
            free(temp_str);
        }

        /* Ok, now we have to add the program and... */
        experiment->argv[experiment->argc] = globals.program_full;
        experiment->argc++;

        /* ...and the program arguments to experiment's argv */
        while ((globals.prog_arg_pos + count) < globals.main_argc) {
            experiment->argv[experiment->argc] =
                globals.main_argv[globals.prog_arg_pos + count];
            experiment->argc++;
            count++;
        }
        experiment->argv[experiment->argc] = NULL;

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

        /* Run program and test return code (should I really test it?) */
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

        /* Run the AFTER program */
        if (NULL != globals.after) {
            argv[0] = globals.after;
            argv[1]; NULL;

            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/after.%d.output", globals.stepdir,
                    input_line);
            experiment->test.output = temp_str;
            test.output = temp_str;
            test.input  = NULL;
            test.info   = globals.after;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _BOLDRED("error running"),
                        globals.after));
            }
            free(temp_str);
        }

        /* Move to the next experiment */
        input_line++;
        experiment = (experiment_t *)perfexpert_list_get_next(experiment);
    }

    CLEANUP:
    /* Free memory */
    // TODO: free list of experiments, and for each experiment argv and test

    return rc;
}

/* run_hpcrun_knc */
static int run_hpcrun_knc(void) {
    const char blank[] = " \t\r\n";
    FILE *script_FP;
    FILE *experiment_FP;
    char file[BUFFER_SIZE];
    char script[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char experiment[BUFFER_SIZE];
    int  experiment_count = 0;
    char *argv[4];
    test_t test;
    int  count = 0;
    
    /* Open experiment file (it is a list of arguments which I use to run) */
    bzero(file, BUFFER_SIZE);
    sprintf(file, "%s/%s", PERFEXPERT_ETCDIR, MIC_EXPERIMENT_FILE);
    if (NULL == (experiment_FP = fopen(file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), file));
        return PERFEXPERT_ERROR;
    }

    /* If this command should run on the MIC, encapsulate it in a script */
    bzero(script, BUFFER_SIZE);
    sprintf(script, "%s/knc_hpcrun.sh", globals.stepdir);

    if (NULL == (script_FP = fopen(script, "w"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), script));
        return PERFEXPERT_ERROR;
    }
    fprintf(script_FP, "#!/bin/sh");

    /* Fill the script file with all the experiments, before, and after */
    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, experiment_FP)) {
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, blank) == strlen(buffer))) {
            continue;
        }

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* In case this is not the first experiment...*/
            if (0 < experiment_count) {
                /* Add the program and the program arguments to experiment */
                fprintf(script_FP, " %s", globals.program_full);
                count = 0;
                while ((globals.prog_arg_pos + count) < globals.main_argc) {
                    fprintf(script_FP, " %s",
                        globals.main_argv[globals.prog_arg_pos + count]);
                    count++;
                }

                /* Add the AFTER program */
                if (NULL != globals.knc_after) {
                    fprintf(script_FP, "\n\n# Run the AFTER command\n");
                    fprintf(script_FP, "%s", globals.knc_after);
                }
            }

            /* Add the BEFORE program */
            if (NULL != globals.knc_before) {
                fprintf(script_FP, "\n\n# Run the BEFORE command\n");
                fprintf(script_FP, "%s", globals.knc_before);
            }

            fprintf(script_FP, "\n\n# Run HPCRUN (%d)\n", experiment_count);
            /* Add PREFIX */
            if (NULL != globals.knc_prefix) {
                fprintf(script_FP, "%s ", globals.knc_prefix);
            }

            /* Arguments to run hpcrun */
            fprintf(script_FP, "%s --output %s/measurements", HPCRUN,
                globals.stepdir);

            /* Move to next line of the experiment file */
            experiment_count++;
            continue;
        }

        /* Remove the \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Add the event */
        fprintf(script_FP, " --event %s", buffer);

        /* Move to next line */
        continue;
    }

    /* Add the program and the program arguments to experiment's */
    count = 0;
    fprintf(script_FP, " %s", globals.program_full);
    while ((globals.prog_arg_pos + count) < globals.main_argc) {
        fprintf(script_FP, " %s", globals.main_argv[globals.prog_arg_pos +
            count]);
        count++;
    }

    /* Add the AFTER program */
    if (NULL != globals.knc_after) {
        fprintf(script_FP, "\n\n# Run the AFTER command\n");
        fprintf(script_FP, "%s", globals.knc_after);
    }
    fprintf(script_FP, "\n\nexit 0\n\n# EOF\n");

    /* Close files and set mode */
    fclose(experiment_FP);
    fclose(script_FP);
    if (-1 == chmod(script, S_IRWXU)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: unable to set script mode"),
            script));
        return PERFEXPERT_ERROR;
    }

    /* The super-ninja test sctructure */
    bzero(file, BUFFER_SIZE);
    sprintf(file, "%s/knc_hpcrun.output", globals.stepdir);

    test.output = file;
    test.input  = NULL;
    test.info   = globals.program;
    
    argv[0] = "ssh";
    argv[1] = globals.knc;
    argv[2] = script;
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        printf("%s    %s %s %s %s\n", PROGRAM_PREFIX, _YELLOW("command line:"),
            argv[0], argv[1], argv[2]);
    }

    /* Run program and test return code (should I really test it?) */
    switch (fork_and_wait(&test, argv)) {
        case PERFEXPERT_ERROR:
            OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
            return PERFEXPERT_ERROR;

        case PERFEXPERT_FAILURE:
        case 255:
            OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
            return PERFEXPERT_FAILURE;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "   [ %s  ]", _BOLDGREEN("OK")));
            return PERFEXPERT_SUCCESS;

        default:
            OUTPUT_VERBOSE((7, "   [UNKNO]"));
            return PERFEXPERT_SUCCESS;
    }
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
