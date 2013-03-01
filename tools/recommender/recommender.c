/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of OptTran and PerfExpert.
 *
 * OptTran as well PerfExpert are free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * OptTran and PerfExpert are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
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
#include <unistd.h>
#include <string.h>
#include <getopt.h>

/* Utility headers */
#include <sqlite3.h>

/* OptTran headers */
#include "config.h"
#include "recommender.h"
#include "opttran_output.h"
#include "opttran_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int recommender_main(int argc, char** argv) {
    opttran_list_t *segments;
    segment_t *item;
    function_t *function;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose          = 0,                 // int
        .verbose_level    = 0,                 // int
        .use_stdin        = 0,                 // int
        .use_stdout       = 1,                 // int
        .use_opttran      = 0,                 // int
        .inputfile        = NULL,              // char *
        .outputfile       = OPTTRAN_RECO_FILE, // char *
        .outputfile_FP    = stdout,            // FILE *
        .dbfile           = NULL,              // char *
        .opttrandir       = NULL,              // char *
        .metrics_file     = NULL,              // char *
        .use_temp_metrics = 0,                 // int
        .colorful         = 0,                 // int
        .source_file      = NULL,              // char *
        .metrics_table    = METRICS_TABLE,     // char *
        .opttran_pid      = (unsigned long long int)getpid(), // int
        .fragments_dir    = NULL,              // char *
        .rec_count        = 3                  // int
    };
    globals.dbfile = (char *)malloc(strlen(RECOMMENDATION_DB) +
                                    strlen(OPTTRAN_VARDIR) + 2);
    if (NULL == globals.dbfile) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    bzero(globals.dbfile,
          strlen(RECOMMENDATION_DB) + strlen(OPTTRAN_VARDIR) + 2);
    sprintf(globals.dbfile, "%s/%s", OPTTRAN_VARDIR, RECOMMENDATION_DB);

    globals.metrics_file = (char *)malloc(strlen(METRICS_FILE) +
                                          strlen(OPTTRAN_ETCDIR) + 2);
    if (NULL == globals.metrics_file) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    bzero(globals.metrics_file,
          strlen(METRICS_FILE) + strlen(OPTTRAN_ETCDIR) + 2);
    sprintf(globals.metrics_file, "%s/%s", OPTTRAN_ETCDIR, METRICS_FILE);

    /* Parse command-line parameters */
    if (OPTTRAN_SUCCESS != parse_cli_params(argc, argv)) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        exit(OPTTRAN_ERROR);
    }
    
    /* Connect to database */
    if (OPTTRAN_SUCCESS != database_connect()) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: connecting to database")));
        exit(OPTTRAN_ERROR);
    }
    
    /* Create the list of code bottlenecks */
    segments = (opttran_list_t *)malloc(sizeof(opttran_list_t));
    if (NULL == segments) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    opttran_list_construct(segments);
    
    /* Define the temporary metrics table name */
    if (1 == globals.use_temp_metrics) {
        globals.metrics_table = (char *)malloc(strlen("metrics_") + 6);
        sprintf(globals.metrics_table, "metrics_%d", (int)getpid());
    }
    
    /* Parse metrics file if 'm' is defined, this will create a temp table */
    if (1 == globals.use_temp_metrics) {
        if (OPTTRAN_SUCCESS != parse_metrics_file()) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: parsing metrics file")));
            exit(OPTTRAN_ERROR);
        }
    }
    
    /* Parse input parameters */
    if (1 == globals.use_stdin) {
        if (OPTTRAN_SUCCESS != parse_segment_params(segments, stdin)) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: parsing input params")));
            exit(OPTTRAN_ERROR);
        }
    } else {
        if (NULL != globals.inputfile) {
            FILE *inputfile_FP = NULL;
            
            /* Open input file */
            if (NULL == (inputfile_FP = fopen(globals.inputfile, "r"))) {
                OPTTRAN_OUTPUT(("%s (%s)",
                                _ERROR("Error: unable to open input file"),
                                globals.inputfile));
                return OPTTRAN_ERROR;
            } else {
                if (OPTTRAN_SUCCESS != parse_segment_params(segments,
                                                            inputfile_FP)) {
                    OPTTRAN_OUTPUT(("%s",
                                    _ERROR("Error: parsing input params")));
                    exit(OPTTRAN_ERROR);
                }
                fclose(inputfile_FP);
            }
        } else {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: undefined input")));
            show_help();
        }
    }

    /* Select/output recommendations for each code bottleneck (5 steps) */
    /* Step 1: Print to a file or STDOUT is ok? Was OPTTRAN chosen? */
    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("STEP 1")));

    if (1 == globals.use_opttran) {
        globals.use_stdout = 0;

        if (NULL == globals.opttrandir) {
            globals.opttrandir = (char *)malloc(strlen("./opttran-") + 8);
            if (NULL == globals.opttrandir) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(globals.opttrandir, strlen("./opttran-" + 8));
            sprintf(globals.opttrandir, "./opttran-%d", getpid());
        }
        OPTTRAN_OUTPUT_VERBOSE((7, "using (%s) as output directory",
                                globals.opttrandir));

        if (OPTTRAN_ERROR == opttran_util_make_path(globals.opttrandir, 0755)) {
            OPTTRAN_OUTPUT(("%s",
                            _ERROR("Error: cannot create opttran directory")));
            exit(OPTTRAN_ERROR);
        }
        
        globals.outputfile = (char *)malloc(strlen(globals.opttrandir) +
                                            strlen(OPTTRAN_RECO_FILE) + 1);
        if (NULL == globals.outputfile) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
            exit(OPTTRAN_ERROR);
        }
        bzero(globals.outputfile, strlen(globals.opttrandir) +
              strlen(OPTTRAN_RECO_FILE) + 1);
        strcat(globals.outputfile, globals.opttrandir);
        strcat(globals.outputfile, "/");
        strcat(globals.outputfile, OPTTRAN_RECO_FILE);
        OPTTRAN_OUTPUT_VERBOSE((7, "printing OPTTRAN output to (%s)",
                                globals.opttrandir));
    }
    if (0 == globals.use_stdout) {
        OPTTRAN_OUTPUT_VERBOSE((7, "printing recommendation to file (%s)",
                                globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OPTTRAN_OUTPUT(("%s (%s)",
                            _ERROR("Error: unable to open output file"),
                            globals.outputfile));
            return OPTTRAN_ERROR;
        }
    } else {
        OPTTRAN_OUTPUT_VERBOSE((7, "printing recommendation to STDOUT"));
    }
#if HAVE_ROSE == 1
    if ((1 == globals.use_opttran) && (NULL != globals.source_file)) {
        globals.fragments_dir = (char *)malloc(strlen(globals.opttrandir) +
                                               strlen(OPTTRAN_FRAGMENTS_DIR) +
                                               10);
        if (NULL == globals.fragments_dir) {
            OPTTRAN_OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
            exit(OPTTRAN_ERROR);
        }
        bzero(globals.fragments_dir, (strlen(globals.opttrandir) +
                                      strlen(OPTTRAN_FRAGMENTS_DIR) + 10));
        sprintf(globals.fragments_dir, "%s/%s", globals.opttrandir,
                OPTTRAN_FRAGMENTS_DIR);
        if (OPTTRAN_ERROR == opttran_util_make_path(globals.fragments_dir,
                                                    0755)) {
            OPTTRAN_OUTPUT(("%s", _ERROR((char *)"Error: cannot create fragments directory")));
            exit(OPTTRAN_ERROR);
        } else {
            OPTTRAN_OUTPUT_VERBOSE((4, "code fragments will be put in (%s)",
                                    globals.fragments_dir));
        }

        /* Open ROSE */
        if (OPTTRAN_ERROR == open_rose()) {
            OPTTRAN_OUTPUT(("%s",
                            _ERROR("Error: starting Rose, disabling OPTTRAN")));
            globals.use_opttran = 0;
        }
    }
#endif

    /* Step 2: For each code bottleneck... */
    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("STEP 2")));

    item = (segment_t *)opttran_list_get_first(segments);
    while ((opttran_list_item_t *)item != &(segments->sentinel)) {
        OPTTRAN_OUTPUT_VERBOSE((4, "%s (%s:%d)",
                                _YELLOW("selecting recommendation for"),
                                item->filename, item->line_number));
        if (1 == globals.use_opttran) {
            fprintf(globals.outputfile_FP, "%% recommendation for %s:%d\n",
                    item->filename, item->line_number);
            fprintf(globals.outputfile_FP, "code.filename=%s\n",
                    item->filename);
            fprintf(globals.outputfile_FP, "code.line_number=%d\n",
                    item->line_number);
            fprintf(globals.outputfile_FP, "code.type=%s\n", item->type);
            fprintf(globals.outputfile_FP, "code.loop_depth=%1.0lf\n",
                item->loop_depth);
        } else {
            fprintf(globals.outputfile_FP,
                    "#--------------------------------------------------\n");
            fprintf(globals.outputfile_FP, "# Recommendations for %s:%d\n",
                    item->filename, item->line_number);
            fprintf(globals.outputfile_FP,
                    "#--------------------------------------------------\n");
        }

        /* Step 3: query DB for recommendations */
        OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("STEP 3")));

        if (OPTTRAN_SUCCESS != select_recommendations(item)) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: selecting recommendations")));
            exit(OPTTRAN_ERROR);
        }
        
        if (0 == globals.use_opttran) {
            fprintf(globals.outputfile_FP, "\n");
        }
        
        /* Step 4: extract fragments */
        OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("STEP 4")));
#if HAVE_ROSE == 1
        if ((1 == globals.use_opttran) && (NULL != globals.source_file)) {
            /* Hey ROSE, here we go... */
            if (OPTTRAN_ERROR == extract_fragment(item)) {
                OPTTRAN_OUTPUT(("%s (%s:%d)",
                                _ERROR("Error: extracting fragments for"),
                                item->filename, item->line_number));
            }
        } else {
            OPTTRAN_OUTPUT_VERBOSE((4, "%s",
                                    _YELLOW("source code not defined, can't extract fragments")));
        }
#endif
        /* Move to the next code bottleneck */
        item = (segment_t *)opttran_list_get_next(item);
    }

    /* Step 5: If we are using an output file, close it! (also close DB) */
    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("STEP 5")));

    if (0 == globals.use_stdout) {
        fclose(globals.outputfile_FP);
    }
    sqlite3_close(globals.db);
#if HAVE_ROSE == 1
    if ((1 == globals.use_opttran) && (NULL != globals.source_file)) {
        /* Output source code */
        if (OPTTRAN_SUCCESS != extract_source()) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: extracting source code")));
            exit(OPTTRAN_ERROR);
        }

        /* Close Rose */
        if (OPTTRAN_SUCCESS != close_rose()) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: closing Rose")));
            exit(OPTTRAN_ERROR);
        }
    }
#endif
    /* Free segments (and all sub-element, including functions) structure */
    while (OPTTRAN_FALSE == opttran_list_is_empty(segments)) {
        item = (segment_t *)opttran_list_get_first(segments);
        opttran_list_remove_item(segments, (opttran_list_item_t *)item);
        free(item->filename);
        free(item->type);
        free(item->extra_info);
        free(item->section_info);
        while (OPTTRAN_FALSE ==
               opttran_list_is_empty((opttran_list_t *)&(item->functions))) {
            function = (function_t *)opttran_list_get_first((opttran_list_t *)&(item->functions));
            free(function->desc);
            opttran_list_item_destruct((opttran_list_item_t *)function);
            free(function);
        }
        opttran_list_item_destruct((opttran_list_item_t *)item);
        free(item); // Some version of GCC will complain about this
    }
    opttran_list_destruct(segments);
    free(segments);
    
    if (1 == globals.use_opttran) {
        free(globals.outputfile);
        if (NULL != globals.source_file) {
            free(globals.fragments_dir);
        }
    }
    if (1 == globals.use_temp_metrics) {
        free(globals.metrics_table);
    }
    // I hope I didn't left anything behind...

    return OPTTRAN_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OPTTRAN_OUTPUT_VERBOSE((10, "printing help"));
    
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: recommender -i|-f file [-o file] [-d database] [-m file] [-nvch] [-p pid]\n");
#if HAVE_ROSE == 1
    printf("                   [-a dir] [-s file] ");
#endif
    printf("                   [-l level]\n");
    printf("  -i --stdin           Use STDIN as input for performance measurements\n");
    printf("  -f --inputfile       Use 'file' as input for performance measurements\n");
    printf("  -o --outputfile      Use 'file' as output for recommendations (default stdout)\n");
    printf("                       if the file exists its content will be overwritten\n");
    printf("  -d --database        Select the recommendation database file\n");
    printf("                       (default: %s/%s)\n", OPTTRAN_VARDIR, RECOMMENDATION_DB);
    printf("  -m --metricfile      Use 'file' to define metrics different from the default\n");
    printf("  -n --newmetrics      Do not use the system metrics table. A temporary table\n");
    printf("                       will be created using the default metrics file:\n");
    printf("                       %s/%s\n", OPTTRAN_ETCDIR, METRICS_FILE);
    printf("  -r --recommendations Number of recommendation to show\n");
#if HAVE_ROSE == 1
    printf("  -a --opttran         Create OptTran (automatic performance optimization) files\n");
    printf("                       into 'dir' directory (default: create no OptTran files).\n");
    printf("                       This argument overwrites -o (no output on STDOUT, except\n");
    printf("                       for verbose messages)\n");
    printf("  -s --sourcefile      Use 'file' to extract source code fragments identified as\n");
    printf("                       bootleneck by PerfExpert (this option sets -a argument)\n");
#endif
    printf("  -p --opttranid       Use 'pid' to identify consecutive calls to Recommender.\n");
    printf("                       This argument is set automatically when using OptTran\n");
    printf("  -v --verbose         Enable verbose mode using default verbose level (5)\n");
    printf("  -l --verbose_level   Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful        Enable colors on verbose mode, no weird characters will\n");
    printf("                       appear on output files\n");
    printf("  -h --help            Show this message\n");

    /* I suppose that if I've to show the help is because something is wrong,
     * or maybe the user just want to see the options, so it seems to be a
     * good idea to exit here with an error code.
     */
    exit(OPTTRAN_ERROR);
}

/* parse_env_vars */
static int parse_env_vars(void) {
    // TODO: add here all the parameter we have for command line arguments
    char *temp_str;
    
    /* Get the variables */
    temp_str = getenv("OPTTRAN_VERBOSE_LEVEL");
    if (NULL != temp_str) {
        globals.verbose_level = atoi(temp_str);
        if (0 != globals.verbose_level) {
            OPTTRAN_OUTPUT_VERBOSE((5, "ENV: verbose_level=%d",
                                       globals.verbose_level));
        }
    }

    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("Environment variables")));

    return OPTTRAN_SUCCESS;
}

/* parse_cli_params */
static int parse_cli_params(int argc, char *argv[]) {
    /** Temporary variable to hold parameter */
    int parameter;
    /** getopt_long() stores the option index here */
    int option_index = 0;

    /* If some environment variable is defined, use it! */
    if (OPTTRAN_SUCCESS != parse_env_vars()) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: parsing environment variables")));
        exit(OPTTRAN_ERROR);
    }

    while (1) {
        /* get parameter */
#if HAVE_ROSE == 1
        parameter = getopt_long(argc, argv, "cvhinm:l:f:d:o:a:s:p:r:",
                                long_options, &option_index);
#else
        parameter = getopt_long(argc, argv, "cvhinm:l:f:d:o:p:r:",
                                long_options, &option_index);
#endif
        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            /* Verbose level */
            case 'l':
                globals.verbose = 1;
                globals.verbose_level = atoi(optarg);
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'l' set"));
                if (0 >= atoi(optarg)) {
                    OPTTRAN_OUTPUT(("%s (%d)",
                                    _ERROR("Error: invalid debug level (too low)"),
                                    atoi(optarg)));
                    show_help();
                }
                if (10 < atoi(optarg)) {
                    OPTTRAN_OUTPUT(("%s (%d)",
                                    _ERROR("Error: invalid debug level (too high)"),
                                    atoi(optarg)));
                    show_help();
                }
                break;

            /* Activate verbose mode */
            case 'v':
                globals.verbose = 1;
                if (0 == globals.verbose_level) {
                    globals.verbose_level = 5;
                }
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'v' set"));
                break;
                
            /* Activate colorful mode */
            case 'c':
                globals.colorful = 1;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Show help */
            case 'h':
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();

            /* Use STDIN? */
            case 'i':
                globals.use_stdin = 1;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'i' set"));
                break;

            /* Use input file? */
            case 'f':
                globals.use_stdin = 0;
                globals.inputfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'f' set [%s]",
                                        globals.inputfile));
                break;

            /* Use output file? */
            case 'o':
                globals.use_stdout = 0;
                globals.outputfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'o' set [%s]",
                                        globals.outputfile));
                break;
#if HAVE_ROSE == 1
            /* Use opttran? */
            case 'a':
                globals.use_opttran = 1;
                globals.use_stdout = 0;
                globals.opttrandir = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'a' set [%s]",
                                        globals.opttrandir));
                break;

            /* Specify the source */
            case 's':
                globals.use_opttran = 1;
                globals.source_file = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 's' set [%s]",
                                        globals.source_file));
                break;
#endif
            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'd' set [%s]",
                                        globals.dbfile));
                break;
                
            /* Use temporary metrics table */
            case 'n':
                globals.use_temp_metrics = 1;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'n' set"));
                break;
                
            /* Specify new metrics */
            case 'm':
                globals.use_temp_metrics = 1;
                globals.metrics_file = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'm' set [%s]",
                                        globals.metrics_file));
                break;
                
            /* Specify OptTran PID */
            case 'p':
                globals.opttran_pid = strtoull(optarg, (char **)NULL, 10);
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'p' set [%llu]",
                                        globals.opttran_pid));
                break;
                
            /* Number of recommendation to output */
            case 'r':
                globals.rec_count = atoi(optarg);
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'r' set [%d]",
                                        globals.rec_count));
                break;
                
            /* Unknown option */
            case '?':
                show_help();
                
            default:
                exit(OPTTRAN_ERROR);
        }
    }
    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("CLI params")));
    OPTTRAN_OUTPUT_VERBOSE((10, "Summary of selected options:"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Verbose:               %s",
                            globals.verbose ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Verbose level:         %d",
                            globals.verbose_level));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Colorful verbose?      %s",
                            globals.colorful ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use STDOUT?            %s",
                            globals.use_stdout ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use STDIN?             %s",
                            globals.use_stdin ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use OPTTRAN?           %s",
                            globals.use_opttran ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Recommendation count:  %d",
                            globals.rec_count));
    OPTTRAN_OUTPUT_VERBOSE((10, "   OPTTRAN PID:           %llu",
                            globals.opttran_pid));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Input file:            %s",
                            globals.inputfile ? globals.inputfile : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Output file:           %s",
                            globals.outputfile ? globals.outputfile : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Database file:         %s",
                            globals.dbfile ? globals.dbfile : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Metrics file:          %s",
                            globals.metrics_file ? globals.metrics_file : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use temporary metrics: %s",
                            globals.use_temp_metrics ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Metrics table:         %s",
                            globals.metrics_table ? globals.metrics_table : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   OPTTRAN directory:     %s",
                            globals.opttrandir ? globals.opttrandir : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Source file:           %s",
                            globals.source_file ? globals.source_file : "(null)"));

    /* Not using OPTTRAN_OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s complete command line:", PROGRAM_PREFIX);
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    return OPTTRAN_SUCCESS;
}

/* parse_metrics_file */
static int parse_metrics_file(void) {
    FILE *metrics_FP;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;

    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s (%s)", _BLUE("Reading metrics file"),
                            globals.metrics_file));

    if (NULL == (metrics_FP = fopen(globals.metrics_file, "r"))) {
        OPTTRAN_OUTPUT(("%s (%s)", _ERROR("Error: unable to open metrics file"),
                        globals.metrics_file));
        show_help();
    } else {
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "CREATE TEMP TABLE %s (\n", globals.metrics_table);
        strcat(sql, "    id INTEGER PRIMARY KEY,\n");
        strcat(sql, "    code_filename CHAR( 1024 ),\n");
        strcat(sql, "    code_line_number INTEGER,\n");
        strcat(sql, "    code_type CHAR( 128 ),\n");
        strcat(sql, "    code_extra_info CHAR( 1024 ),\n");

        bzero(buffer, BUFFER_SIZE);
        while (NULL != fgets(buffer, BUFFER_SIZE - 1, metrics_FP)) {
            int temp;
            
            /* avoid comments */
            if ('#' == buffer[0]) {
                continue;
            }
            /* avoid blank and too short lines */
            if (10 >= strlen(buffer)) {
                continue;
            }
            /* replace some characters just to provide a safe SQL clause */
            for (temp = 0; temp < strlen(buffer); temp++) {
                switch (buffer[temp]) {
                    case '%':
                    case '.':
                    case '(':
                    case ')':
                    case '-':
                    case ':':
                        buffer[temp] = '_';
                    default:
                        break;
                }
            }
            buffer[strcspn(buffer, "\n")] = '\0'; // remove the '\n'
            strcat(sql, "    ");
            strcat(sql, buffer);
            strcat(sql, " FLOAT,\n");
        }
        sql[strlen(sql)-2] = '\0'; // remove the last ',' and '\n'
        strcat(sql, ");");
        OPTTRAN_OUTPUT_VERBOSE((10, "metrics SQL: %s", _CYAN(sql)));

        /* Create metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, 0,
                                      &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            fprintf(stderr, "SQL clause: %s\n", sql);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
        OPTTRAN_OUTPUT(("using temporary metric table (%s)",
                        globals.metrics_table));
    }
    return OPTTRAN_SUCCESS;
}

/* parse_segment_params */
static int parse_segment_params(opttran_list_t *segments_p, FILE *inputfile_p) {
    segment_t *item;
    int  input_line = 0;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    int  rowid = 0;

    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing measurements")));
    
    /* Which INPUT we are using? (just a double check) */
    if ((NULL == inputfile_p) && (globals.use_stdin)) {
        inputfile_p = stdin;
    }
    if (globals.use_stdin) {
        OPTTRAN_OUTPUT_VERBOSE((3,
                                "using STDIN as input for perf measurements"));
    } else {
        OPTTRAN_OUTPUT_VERBOSE((3, "using (%s) as input for perf measurements",
                                globals.inputfile));
    }

    /* For each line in the INPUT file... */
    OPTTRAN_OUTPUT_VERBOSE((7, "--- parsing input file"));

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "BEGIN TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, inputfile_p)) {
        node_t *node;
        int temp;
        
        input_line++;

        /* Ignore comments */
        if (0 == strncmp("#", buffer, 1)) {
            continue;
        }

        /* Is this line a new code bottleneck specification? */
        if (0 == strncmp("%", buffer, 1)) {
            char temp_str[BUFFER_SIZE];
            
            OPTTRAN_OUTPUT_VERBOSE((5, "(%d) --- %s", input_line,
                                    _GREEN("new bottleneck found")));

            /* Create a list item for this code bottleneck */
            item = (segment_t *)malloc(sizeof(segment_t));
            if (NULL == item) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *) item);
            
            /* Initialize some elements on segment */
            item->filename = NULL;
            item->line_number = 0;
            item->type = NULL;
            item->extra_info = NULL;
            item->section_info = NULL;
            item->loop_depth = 0;
            item->representativeness = 0;
            item->rowid = 0;
            item->outer_loop = 0;
            item->outer_outer_loop = 0;

            /* Add this item to 'segments' */
            opttran_list_append(segments_p, (opttran_list_item_t *) item);

            /* Create the SQL statement for this new segment */
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                    "INSERT INTO %s (code_filename) VALUES ('new_code-%d');\n",
                    globals.metrics_table, (int)getpid());
            bzero(sql, BUFFER_SIZE);
            strcat(sql, temp_str);
            strcat(sql, "                           "); // Pretty print ;-)
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                    "SELECT id FROM %s WHERE code_filename = 'new_code-%d';",
                    globals.metrics_table, (int)getpid());
            strcat(sql, temp_str);

            OPTTRAN_OUTPUT_VERBOSE((5, "        SQL: %s", _CYAN(sql)));
            
            /* Insert new code fragment into metrics database, retrieve id */
            if (SQLITE_OK != sqlite3_exec(globals.db, sql, get_rowid,
                                          (void *)&rowid, &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                sqlite3_close(globals.db);
                exit(OPTTRAN_ERROR);
            } else {
                OPTTRAN_OUTPUT_VERBOSE((5, "        ID: %d", rowid));
                /* Store the rowid on the segment structure */
                item->rowid = rowid;
            }

            /* Set OptTran PID for the new segment */
            bzero(sql, BUFFER_SIZE);
            sprintf(sql, "UPDATE %s SET pid=%llu WHERE id=%d;",
                    globals.metrics_table, globals.opttran_pid, rowid);
            if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                          &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                sqlite3_close(globals.db);
                exit(OPTTRAN_ERROR);
            }

            continue;
        }

        node = (node_t *)malloc(sizeof(node_t) + strlen(buffer) + 1);
        if (NULL == node) {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
            exit(OPTTRAN_ERROR);
        }
        bzero(node, sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments' (only code.* parameters) other parameter should fit into
         * the SQL DB.
         */

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            item->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->filename) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(item->filename, strlen(node->value) + 1);
            strcpy(item->filename, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) %s     [%s]", input_line,
                                    _MAGENTA("filename:"), item->filename));
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            item->line_number = atoi(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) %s  [%d]", input_line,
                                    _MAGENTA("line number:"),
                                    item->line_number));
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            item->type = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->type) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(item->type, strlen(node->value) + 1);
            strcpy(item->type, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) type:         [%s]",
                                    input_line, item->type));
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            item->extra_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->extra_info) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(item->extra_info, strlen(node->value) + 1);
            strcpy(item->extra_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) extra info:   [%s]", input_line,
                                    item->extra_info));
        }
        /* Code param: code.representativeness */
        if (0 == strncmp("code.representativeness", node->key, 23)) {
            item->representativeness = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) representativeness: [%f], ",
                                    input_line, item->representativeness));
            free(node);
            continue;
        }
        /* Code param: code.section_info */
        if (0 == strncmp("code.section_info", node->key, 17)) {
            item->section_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->section_info) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(item->section_info, strlen(node->value) + 1);
            strcpy(item->section_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) section info: [%s]", input_line,
                                    item->section_info));
            free(node);
            continue;
        }
        /* Code param: perfexpert.loop_depth */
        if (0 == strncmp("perfexpert.loop-depth", node->key, 21)) {
            item->loop_depth = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) loop depth: [%f]", input_line,
                                    item->loop_depth));
        }

        /* Clean the node->key (remove undesired characters) */
        for (temp = 0; temp < strlen(node->key); temp++) {
            switch (node->key[temp]) {
                case '%':
                case '.':
                case '(':
                case ')':
                case '-':
                case ':':
                    node->key[temp] = '_';
                default:
                    break;
            }
        }
        
        /* Assemble the SQL query */
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "UPDATE %s SET %s='%s' WHERE id=%d;",
                globals.metrics_table, node->key, node->value, rowid);

        /* Update metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                      &error_msg)) {
            OPTTRAN_OUTPUT_VERBOSE((4, "(%d) %s (%s = %s)", input_line,
                                    _RED("ignored line"), node->key,
                                    node->value));
            sqlite3_free(error_msg);
        } else {
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) %s", input_line, sql));
        }
        free(node);
    }

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "END TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    /* print a summary of 'segments' */
    OPTTRAN_OUTPUT_VERBOSE((4, "%d %s", opttran_list_get_size(segments_p),
                            _GREEN("code segment(s) found")));

    item = (segment_t *)opttran_list_get_first(segments_p);
    while ((opttran_list_item_t *)item != &(segments_p->sentinel)) {
        OPTTRAN_OUTPUT_VERBOSE((4, "   %s:%d", item->filename,
                                item->line_number));
        item = (segment_t *)opttran_list_get_next(item);
    }
    
    OPTTRAN_OUTPUT_VERBOSE((4, "==="));
    return OPTTRAN_SUCCESS;
}

/* get_rowid */
static int get_rowid(void *rowid, int col_count, char **col_values,
                     char **col_names) {
    int *temp = (int *)rowid;
    if (NULL != col_values[0]) {
        *temp = atoi(col_values[0]);
    }
    return OPTTRAN_SUCCESS;
}

/* get_rowid */
static int get_weight(void *weight, int col_count, char **col_values,
                      char **col_names) {
    double *temp = (double *)weight;
    if (NULL != col_values[0]) {
        *temp = atof(col_values[0]);
    }
    return OPTTRAN_SUCCESS;
}

/* output_patterns */
static int output_patterns(void *weight, int col_count, char **col_values,
                           char **col_names) {
    if (0 == globals.use_opttran) {
        /* Pretty print for the user */
        if (NULL != col_values[0]) {
            fprintf(globals.outputfile_FP, "%s ", col_values[0]);
        }
    } else {
        /* OptTran output */
        if ((NULL != col_values[0]) && (NULL != col_values[1])) {
            fprintf(globals.outputfile_FP, "recommender.recognizer_id=%s\n",
                    col_values[1]);
            fprintf(globals.outputfile_FP, "recommender.recognizer=%s\n",
                    col_values[0]);
        }
    }
    return OPTTRAN_SUCCESS;
}

/* output_recommendations */
static int output_recommendations(void *not_used, int col_count,
                                  char **col_values, char **col_names) {
    char temp_str[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    /*
     * 0 (TEXT)    desc
     * 1 (TEXT)    reason
     * 2 (INTEGER) id
     * 3 (TEXT)    example
     */
    OPTTRAN_OUTPUT_VERBOSE((7, "%s", _GREEN("new recommendation found")));
    
    if (0 == globals.use_opttran) {
        /* Pretty print for the user */
        fprintf(globals.outputfile_FP, "#\n# This is a possible recommendation");
        fprintf(globals.outputfile_FP, " for this code segment\n#\n");
        fprintf(globals.outputfile_FP, "Recommendation ID: %s\n",
                col_values[2]);
        fprintf(globals.outputfile_FP, "Recommendation Description: %s\n",
                col_values[0]);
        fprintf(globals.outputfile_FP, "Recommendation Reason: %s\n",
                col_values[1]);
        fprintf(globals.outputfile_FP, "Pattern Recognizers: ");

        /* Find the patterns available for this recommendation id */
        bzero(sql, BUFFER_SIZE);
        strcat(sql, "SELECT p.recognizer FROM pattern AS p INNER JOIN");
        strcat(sql, "\n                        ");
        strcat(sql, "recommendation_pattern AS rp ON p.id = rp.id_pattern");
        strcat(sql, "\n                        ");
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "WHERE rp.id_recommendation = %s;", col_values[2]);
        strcat(sql, temp_str);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_patterns, NULL,
                                      &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
        fprintf(globals.outputfile_FP, "\n");
        fprintf(globals.outputfile_FP, "Code example:\n%s\n", col_values[3]);
    } else {
        /* OptTran output */
        fprintf(globals.outputfile_FP, "recommender.recommendation_id=%s\n",
                col_values[2]);
        /* Find the patterns available for this recommendation id */
        bzero(sql, BUFFER_SIZE);
        strcat(sql, "SELECT p.recognizer, p.id FROM pattern AS p INNER JOIN");
        strcat(sql, "\n                        ");
        strcat(sql, "recommendation_pattern AS rp ON p.id = rp.id_pattern");
        strcat(sql, "\n                        ");
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "WHERE rp.id_recommendation = %s;", col_values[2]);
        strcat(sql, temp_str);
        
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_patterns, NULL,
                                      &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
    }
    return OPTTRAN_SUCCESS;
}

/* database_connect */
static int database_connect(void) {
    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("Connecting to database")));

    /* Connect to the DB */
    if (NULL == globals.dbfile) {
        globals.dbfile = "./recommendation.db";
    }
    if (-1 == access(globals.dbfile, F_OK)) {
        OPTTRAN_OUTPUT(("%s (%s)",
                        _ERROR("Error: recommendation database doesn't exist"),
                        globals.dbfile));
        return OPTTRAN_ERROR;
    }
    if (-1 == access(globals.dbfile, R_OK)) {
        OPTTRAN_OUTPUT(("%s (%s)",
                        _ERROR("Error: you don't have permission to read"),
                        globals.dbfile));
        return OPTTRAN_ERROR;
    }
    
    if (SQLITE_OK != sqlite3_open(globals.dbfile, &(globals.db))) {
        OPTTRAN_OUTPUT(("%s (%s), %s", _ERROR("Error: openning database"),
                        globals.dbfile, sqlite3_errmsg(globals.db)));
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    } else {
        OPTTRAN_OUTPUT_VERBOSE((4, "connected to %s", globals.dbfile));
    }
    return OPTTRAN_SUCCESS;
}

/* accumulate_functions */
static int accumulate_functions(void *functions, int col_count,
                                char **col_values, char **col_names) {
    function_t *function;
    
    /* Copy SQL query result into functions list */
    function = (function_t *)malloc(sizeof(function_t));
    if (NULL == function) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    opttran_list_item_construct((opttran_list_item_t *) function);

    function->id = atoi(col_values[0]);
    
    function->desc = (char *)malloc(strlen(col_values[1]) + 1);
    if (NULL == function->desc) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    bzero(function->desc, strlen(col_values[1]) + 1);
    strncpy(function->desc, col_values[1], strlen(col_values[1]));
    
    bzero(function->statement, BUFFER_SIZE);
    strncpy(function->statement, col_values[2], strlen(col_values[2]));

    opttran_list_append((opttran_list_t *) functions,
                        (opttran_list_item_t *) function);

    OPTTRAN_OUTPUT_VERBOSE((10, "%s (ID: %d, %s) [%d bytes]",
                            _YELLOW("function found"), function->id,
                            function->desc, strlen(function->statement)));

    return OPTTRAN_SUCCESS;
}

/* select_recommendations */
static int select_recommendations(segment_t *segment) {
    char *error_msg = NULL;
    function_t *function;
    char sql[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE];
    double weight = 0;

    opttran_list_construct((opttran_list_t *) &(segment->functions));
    
    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("Querying recommendation DB")));
    
    /* Select all functions, accumulate them */
    OPTTRAN_OUTPUT_VERBOSE((8, "%s [%s:%d, ROWID: %d]",
                            _YELLOW("looking for functions to segment"),
                            segment->filename, segment->line_number,
                            segment->rowid));
    
    if (SQLITE_OK != sqlite3_exec(globals.db,
                                  "SELECT id, desc, statement FROM function;",
                                  accumulate_functions,
                                  (void *)&(segment->functions),
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    /* Execute all functions, accumulate recommendations */
    OPTTRAN_OUTPUT_VERBOSE((8, "%d %s",
                            opttran_list_get_size(&(segment->functions)),
                            _GREEN("functions found")));

    /* Create a temporary table to store possible recommendations */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "CREATE TABLE recommendation_%d_%d (function_id INTEGER,\n",
            (int)getpid(), segment->rowid);
    strcat(sql, "                        ");
    strcat(sql, "recommendation_id INTEGER, score FLOAT, weigth FLOAT);");

    OPTTRAN_OUTPUT_VERBOSE((10, "%s",
                            _YELLOW("creating temporary table of recommendations")));
    OPTTRAN_OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));
    
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    /* For each function, execute it! */
    function = (function_t *)opttran_list_get_first(&(segment->functions));
    while ((opttran_list_item_t *)function != &(segment->functions.sentinel)) {
        sqlite3_stmt *statement;
        int rc;
        
        OPTTRAN_OUTPUT_VERBOSE((8, "%s (ID: %d, %s) [%d bytes]",
                                _YELLOW("running function"), function->id,
                                function->desc, strlen(function->statement)));

        /* Prepare the SQL statement */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, function->statement,
                                            BUFFER_SIZE, &statement, NULL)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }

        /* Bind ROWID */
        if (SQLITE_OK != sqlite3_bind_int(statement,
                                          sqlite3_bind_parameter_index(statement, "@RID"),
                                          segment->rowid)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }

        /* Bind loop depth */
        if (SQLITE_OK != sqlite3_bind_double(statement,
                                             sqlite3_bind_parameter_index(statement, "@LPD"),
                                             segment->loop_depth)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }

        /* Run que query */
        OPTTRAN_OUTPUT_VERBOSE((10, "   %s",
                                _GREEN("list of recommendations: ")));
        do {
            rc = sqlite3_step(statement);

            /* It is possible that this function does not return results */
            if (SQLITE_DONE == rc) {
                OPTTRAN_OUTPUT_VERBOSE((10, "      end of recommendations for function (%d)",
                                        function->id));
                continue;
            } else if (SQLITE_ROW == rc) {
                /* check if there are at least two columns in the SQL result and
                 * their types are SQLITE_INTEGER and SQLITE_FLOAT respectivelly
                 */
                if (SQLITE_INTEGER != sqlite3_column_type(statement, 0)) {
                    OPTTRAN_OUTPUT(("%s", _ERROR("The first column of a recommendation function returned a type different than double. The result of this function will be ignored.")));
                    continue;
                }
                if (SQLITE_FLOAT != sqlite3_column_type(statement, 1)) {
                    OPTTRAN_OUTPUT(("%s", _ERROR("The second column of a recommendation function returned a type different than double. The result of this function will be ignored.")));
                    continue;
                }
                
                /* Consider only the results where the score is positive */
                if (0 < sqlite3_column_double(statement, 1)) {
                    /* Find the weigth of this recommendation */
                    weight = 0;
                    bzero(sql, BUFFER_SIZE);
                    strcat(sql, "SELECT weight FROM recommendation_function WHERE");
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str, " id_recommendation=%d AND id_function=%d;",
                            sqlite3_column_int(statement, 0), function->id);
                    strcat(sql, temp_str);
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql, get_weight,
                                                  (void *)&weight, &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        sqlite3_close(globals.db);
                        exit(OPTTRAN_ERROR);
                    }
                    
                    /* Insert recommendation into the temporary table */
                    bzero(sql, BUFFER_SIZE);
                    sprintf(sql, "INSERT INTO recommendation_%d_%d ", (int)getpid(),
                            segment->rowid);
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str, "VALUES (%d, %d, %f, %f);", function->id,
                            sqlite3_column_int(statement, 0),
                            sqlite3_column_double(statement, 1), weight);
                    strcat(sql, temp_str);
                    
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                                  &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        sqlite3_close(globals.db);
                        exit(OPTTRAN_ERROR);
                    }
                    
                    OPTTRAN_OUTPUT_VERBOSE((10, "      FunctionID=%d, RecommendationID=%d, Score=%f, Weight=%f",
                                            function->id,
                                            sqlite3_column_int(statement, 0),
                                            sqlite3_column_double(statement, 1),
                                            weight));
                }
            }
        } while (SQLITE_ROW == rc);

        /* Something went wrong :-/ */
        if (SQLITE_DONE != rc) {
            fprintf(stderr, "Error: SQL error: %s\n",
                    sqlite3_errmsg(globals.db));
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
        
        /* SQLite3 cleanup */
        if (SQLITE_OK != sqlite3_reset(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
        if (SQLITE_OK != sqlite3_finalize(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            sqlite3_close(globals.db);
            exit(OPTTRAN_ERROR);
        }
        
        /* Move to the next code bottleneck */
        function = (function_t *)opttran_list_get_next(function);
    }

    /* Calculate the normalized rank of recommendations, should use weights? */
    // TODO: someday, I will add the weighting system here

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, BUFFER_SIZE);
    strcat(sql, "SELECT r.desc AS desc, r.reason AS reason, r.id AS id,");
    strcat(sql, "\n                        ");
    strcat(sql, "r.example AS example FROM recommendation AS r");
    strcat(sql, "\n                        ");
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str,
            "INNER JOIN recommendation_%d_%d AS m ON r.id = m.recommendation_id",
            (int)getpid(), segment->rowid);
    strcat(sql, temp_str);
    strcat(sql, "\n                        ");
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "ORDER BY m.score DESC LIMIT %d;", globals.rec_count);
    strcat(sql, temp_str);
    
    OPTTRAN_OUTPUT_VERBOSE((10, "%s (top %d)",
                            _YELLOW("selecting top-N recommendations"),
                            globals.rec_count));
    OPTTRAN_OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_recommendations, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    /* Remove the temporary table now, don't wait for the connection closing */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "DROP TABLE recommendation_%d_%d;", (int)getpid(),
            segment->rowid);
    
    OPTTRAN_OUTPUT_VERBOSE((10, "%s",
                            _YELLOW("dropping temporary table of recommendations")));
    OPTTRAN_OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));
    
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    OPTTRAN_OUTPUT_VERBOSE((7, "==="));
    
    return OPTTRAN_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
