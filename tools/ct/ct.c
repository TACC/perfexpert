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
#include <inttypes.h>
#include <getopt.h>
#include <fcntl.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "config.h"
#include "ct.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int ct_main(int argc, char** argv) {
    perfexpert_list_t *fragments;
    fragment_t *fragment;
    recommendation_t *recommendation;
    int rc = PERFEXPERT_NO_TRANS;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level    = 0,      // int
        .inputfile        = NULL,   // char *
        .inputfile_FP     = stdin,   // char *
        .outputfile       = NULL,   // char *
        .outputfile_FP    = stdout, // FILE *
        .dbfile           = NULL,   // char *
        .workdir          = NULL,   // char *
        .perfexpert_pid   = (unsigned long long int)getpid(), // int
        .colorful         = 0       // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create the list of fragments */
    fragments = (perfexpert_list_t *)malloc(sizeof(perfexpert_list_t));
    if (NULL == fragments) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_construct(fragments);

    /* Open input file */
    if (NULL != globals.inputfile) {
        if (NULL == (globals.inputfile_FP = fopen(globals.inputfile, "r"))) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open input file"),
                    globals.inputfile));
            return PERFEXPERT_ERROR;
        }
    }

    /* Print to a file or STDOUT is? */
    if (NULL != globals.outputfile) {
        OUTPUT_VERBOSE((7, "printing recommendations to file (%s)",
                        globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                    globals.outputfile));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }
    } else {
        OUTPUT_VERBOSE((7, "printing recommendations to STDOUT"));
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
                                                          globals.dbfile)) {
        OUTPUT(("%s", _ERROR("Error: connecting to database")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }

    /* Parse input parameters */
    if (PERFEXPERT_SUCCESS != parse_transformation_params(fragments)) {
        OUTPUT(("%s", _ERROR("Error: parsing input params")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }

    /* Select transformations for each code bottleneck */
    fragment = (fragment_t *)perfexpert_list_get_first(fragments);
    while ((perfexpert_list_item_t *)fragment != &(fragments->sentinel)) {

        /* Query DB for transformations */
        if (PERFEXPERT_SUCCESS != select_transformations(fragment)) {
            OUTPUT(("%s", _ERROR("Error: applying transformation")));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }

        /* Apply recommendation (actually only one will be applied) */
        switch (apply_recommendations(fragment)) {
            case PERFEXPERT_NO_TRANS:
                OUTPUT(("%s", _GREEN("Sorry, we have no transformations")));
                /* Move to the next code bottleneck */
                fragment = (fragment_t *)perfexpert_list_get_next(fragment);
                continue;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            default: 
                OUTPUT(("%s", _ERROR("Error: selecting transformations")));
                rc = PERFEXPERT_ERROR;
                goto cleanup;
        }

        /* Move to the next code bottleneck */
        fragment = (fragment_t *)perfexpert_list_get_next(fragment);
    }

    cleanup:
    /* Close files and DB connection */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    perfexpert_database_disconnect(globals.db);
    return PERFEXPERT_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));

    /*               12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    fprintf(stderr, "Usage: perfexpert_ct -f file [-o file] [-vch] [-l level] [-a workdir] [-p pid]");
    fprintf(stderr, "                     [-d database]\n\n");
    fprintf(stderr, "  -f --inputfile       Use 'file' as input for patterns\n");
    fprintf(stderr, "  -o --outputfile      Use 'file' as output (default stdout)\n");
    fprintf(stderr, "  -a --automatic       Use automatic performance optimization and create files\n");
    fprintf(stderr, "                       into 'dir' directory (default: off).\n");
    fprintf(stderr, "  -d --database        Select the recommendation database file\n");
    fprintf(stderr, "                       (default: %s/%s)\n", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
    fprintf(stderr, "  -p --perfexpert_pid  Use 'pid' to log on DB consecutive calls to Recommender\n");
    fprintf(stderr, "  -v --verbose         Enable verbose mode using default verbose level (5)\n");
    fprintf(stderr, "  -l --verbose_level   Enable verbose mode using a specific verbose level (1-10)\n");
    fprintf(stderr, "  -c --colorful        Enable colors on verbose mode, no weird characters will\n");
    fprintf(stderr, "                       appear on output files\n");
    fprintf(stderr, "  -h --help            Show this message\n");
}

/* parse_env_vars */
static int parse_env_vars(void) {
    char *temp_str;
    
    temp_str = getenv("PERFEXPERT_CT_VERBOSE_LEVEL");
    if (NULL != temp_str) {
        if ((0 >= atoi(temp_str)) || (10 < atoi(temp_str))) {
            OUTPUT(("%s (%d)", _ERROR("ENV Error: invalid debug level"),
                    atoi(temp_str)));
            show_help();
            return PERFEXPERT_ERROR;
        }
        globals.verbose_level = atoi(temp_str);
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
    }

    temp_str = getenv("PERFEXPERT_CT_INPUT_FILE");
    if (NULL != temp_str) {
        globals.inputfile = temp_str;
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.inputfile));
    }

    temp_str = getenv("PERFEXPERT_CT_OUTPUT_FILE");
    if (NULL != temp_str) {
        globals.outputfile = temp_str;
        OUTPUT_VERBOSE((5, "ENV: outputfile=%s", globals.outputfile));
    }

    temp_str = getenv("PERFEXPERT_CT_DATABASE_FILE");
    if (NULL != temp_str) {
        globals.dbfile =  temp_str;
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }

    temp_str = getenv("PERFEXPERT_CT_WORKDIR");
    if (NULL != temp_str) {
        globals.workdir = temp_str;
        OUTPUT_VERBOSE((5, "ENV: workdir=%s", globals.workdir));
    }

    temp_str = getenv("PERFEXPERT_CT_COLORFUL");
    if ((NULL != temp_str) && (1 == atoi(temp_str))) {
        globals.colorful = 1;
        OUTPUT_VERBOSE((5, "ENV: colorful=YES"));
    }

    return PERFEXPERT_SUCCESS;
}

/* parse_cli_params */
static int parse_cli_params(int argc, char *argv[]) {
    /** Temporary variable to hold parameter */
    int parameter;
    /** getopt_long() stores the option index here */
    int option_index = 0;

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

            /* Specify PerfExpert PID */
            case 'p':
                globals.perfexpert_pid = strtoull(optarg, (char **)NULL, 10);
                OUTPUT_VERBOSE((10, "option 'p' set [%llu]",
                                globals.perfexpert_pid));
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
    OUTPUT_VERBOSE((10, "   Verbose level:              %d",
                    globals.verbose_level));
    OUTPUT_VERBOSE((10, "   Colorful verbose?           %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Input file:                 %s",
                    globals.inputfile ? globals.inputfile : "(null)"));
    OUTPUT_VERBOSE((10, "   Output file:                %s",
                    globals.outputfile ? globals.outputfile : "(null)"));
    OUTPUT_VERBOSE((10, "   PerfExpert PID:             %llu",
                    globals.perfexpert_pid));
    OUTPUT_VERBOSE((10, "   Temporary directory:        %s",
                    globals.workdir ? globals.workdir : "(null)"));
    OUTPUT_VERBOSE((10, "   Database file:              %s",
                    globals.dbfile ? globals.dbfile : "(null)"));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s complete command line:", PROGRAM_PREFIX);
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    return PERFEXPERT_SUCCESS;
}

/* parse_transformation_params */
static int parse_transformation_params(perfexpert_list_t *fragments) {
    fragment_t *fragment;
    recommendation_t *recommendation;
    char buffer[BUFFER_SIZE];
    int  input_line = 0;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing fragments")));

    /* Which INPUT we are using? (just a double check) */
    if (NULL == globals.inputfile) {
        OUTPUT_VERBOSE((3, "using STDIN as input for perf measurements"));
    } else {
        OUTPUT_VERBOSE((3, "using (%s) as input for perf measurements",
                        globals.inputfile));
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, globals.inputfile_FP)) {
        node_t *node;
        int temp;

        input_line++;

        /* Ignore comments */
        if (0 == strncmp("#", buffer, 1)) {
            continue;
        }

        /* Is this line a new recommendation? */
        if (0 == strncmp("%", buffer, 1)) {
            char temp_str[BUFFER_SIZE];

            OUTPUT_VERBOSE((5, "(%d) --- %s", input_line,
                            _GREEN("new bottleneck found")));

            /* Create a list item for this code fragment */
            fragment = (fragment_t *)malloc(sizeof(fragment_t));
            if (NULL == fragment) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)fragment);
            perfexpert_list_construct(
                (perfexpert_list_t *)&(fragment->recommendations));
            perfexpert_list_append(fragments,
                                   (perfexpert_list_item_t *)fragment);

            fragment->fragment_file = NULL;
            fragment->outer_loop_fragment_file = NULL;
            fragment->outer_outer_loop_fragment_file = NULL;
            fragment->outer_loop_line_number = 0;
            fragment->outer_outer_loop_line_number = 0;

            continue;
        }

        node = (node_t *)malloc(sizeof(node_t) + strlen(buffer) + 1);
        if (NULL == node) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(node, sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            fragment->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->filename) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(fragment->filename, strlen(node->value) + 1);
            strcpy(fragment->filename, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%s]", input_line,
                            _MAGENTA("filename:"), fragment->filename));
            free(node);
            continue;
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            fragment->line_number = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%d]", input_line,
                            _MAGENTA("line number:"), fragment->line_number));
            free(node);
            continue;
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            fragment->code_type = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->code_type) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(fragment->code_type, strlen(node->value) + 1);
            strcpy(fragment->code_type, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%s]", input_line, _MAGENTA("type:"),
                            fragment->code_type));
            free(node);
            continue;
        }
        /* Code param: code.function_name */
        if (0 == strncmp("code.function_name", node->key, 18)) {
            fragment->function_name = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->function_name) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(fragment->function_name, strlen(node->value) + 1);
            strcpy(fragment->function_name, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%s]", input_line,
                            _MAGENTA("function name:"),
                            fragment->function_name));
            free(node);
            continue;
        }
        /* Code param: code.loop_depth */
        if (0 == strncmp("code.loop_depth", node->key, 15)) {
            fragment->loop_depth = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%d]", input_line,
                            _MAGENTA("loop depth:"), fragment->loop_depth));
            free(node);
            continue;
        }
        /* Code param: code.rowid */
        if (0 == strncmp("code.rowid", node->key, 10)) {
            fragment->rowid = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%d]", input_line,
                            _MAGENTA("row id:"), fragment->rowid));
            free(node);
            continue;
        }
        /* Code param: recommender.recommendation_id */
        if (0 == strncmp("recommender.recommendation_id", node->key, 29)) {
            recommendation = (recommendation_t *)malloc(sizeof(
                recommendation_t));
            if (NULL == recommendation) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)recommendation);
            perfexpert_list_append(
                (perfexpert_list_t *)&(fragment->recommendations),
                (perfexpert_list_item_t *)recommendation);

            recommendation->id = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) >> %s [%d]", input_line,
                            _YELLOW("recommendation id:"),
                            recommendation->id));
            free(node);
            continue;
        }

        /* Should never reach this point, only if there is garbage in the input
         * file.
         */
        OUTPUT_VERBOSE((4, "(%d) %s (%s = %s)", input_line,
                        _RED("ignored line"), node->key, node->value));
        free(node);
    }

    /* print a summary of 'fragments' */
    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(fragments),
                    _GREEN("code bottleneck(s) found")));

    return PERFEXPERT_SUCCESS;
}

/* select_transformations */
static int select_transformations(fragment_t *fragment) {
    recommendation_t *recommendation;
    char sql[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE];
    char *error_msg = NULL;

    OUTPUT_VERBOSE((7, "=== %s", _BLUE("Querying DB")));

    /* For each recommendation in this fragment, select the transformers */
    recommendation = (recommendation_t *)perfexpert_list_get_first(
        &(fragment->recommendations));

    while ((perfexpert_list_item_t *)recommendation !=
        &(fragment->recommendations.sentinel)) {
        OUTPUT_VERBOSE((4, "   %s [%d]",
                        _YELLOW("selecting transformations for recommendation"),
                        recommendation->id));

        /* Initialyze the list of transformations */
        perfexpert_list_construct(&(recommendation->transformations));

        /* Find the transformations available for this recommendation */
        bzero(sql, BUFFER_SIZE);
        strcat(sql, "SELECT t.transformer, t.id FROM transformation AS t ");
        strcat(sql, "INNER JOIN recommendation_transformation AS rt ");
        strcat(sql, "ON t.id = rt.id_transformation ");
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "WHERE rt.id_recommendation = %d;",
            recommendation->id);
        strcat(sql, temp_str);
        OUTPUT_VERBOSE((10, "      SQL: %s", _CYAN(sql)));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                                      accumulate_transformations,
                                      (void *)recommendation, &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Move to the next code recommendation */
        recommendation = (recommendation_t *)perfexpert_list_get_next(
            (perfexpert_list_item_t *)recommendation);
    }
    return PERFEXPERT_SUCCESS;
}

/* accumulate_transformations */
static int accumulate_transformations(void *recommendation, int count,
    char **val, char **names) {
    recommendation_t *r;
    transformation_t *transformation;
    char sql[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE];
    char *error_msg = NULL;

    /* Create the transformation item */
    transformation = (transformation_t *)malloc(sizeof(transformation_t));
    if (NULL == transformation) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)transformation);

    /* Set the transformation ID */
    transformation->id = atoi(val[1]);

    /* Set the transformer, aka program */
    transformation->program = (char *)malloc(strlen(val[0]) + 1);
    bzero(transformation->program, strlen(val[0]) + 1);
    strncpy(transformation->program, val[0], strlen(val[0]));

    OUTPUT_VERBOSE((7, "      %s [%d]", _GREEN(transformation->program),
                    transformation->id));

    OUTPUT_VERBOSE((4, "      %s [%d]",
                    _YELLOW("selecting patterns for transformation"),
                    transformation->id));

    /* Initialize the list of pattern recognizers */
    perfexpert_list_construct(&(transformation->patterns));

    /* Find the pattern recognizers available for this transformation */
    bzero(sql, BUFFER_SIZE);
    strcat(sql, "SELECT p.recognizer, p.id FROM pattern AS p ");
    strcat(sql, "INNER JOIN transformation_pattern AS tp ");
    strcat(sql, "ON p.id = tp.id_pattern ");
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "WHERE tp.id_transformation = %d;", transformation->id);
    strcat(sql, temp_str);
    OUTPUT_VERBOSE((10, "         SQL: %s", _CYAN(sql)));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, accumulate_patterns,
                                  (void *)transformation, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* Add this transformation to the recommendation list */
    r = (recommendation_t *)recommendation;
    perfexpert_list_append((perfexpert_list_t *)&(r->transformations),
                           (perfexpert_list_item_t *)transformation);

    return PERFEXPERT_SUCCESS;
}

/* accumulate_patterns */
static int accumulate_patterns(void *transformation, int count, char **val,
    char **names) {
    transformation_t *t;
    pattern_t *pattern;

    /* Create the pattern item */
    pattern = (pattern_t *)malloc(sizeof(pattern_t));
    if (NULL == pattern) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)pattern);

    /* Set the pattern ID */
    pattern->id = atoi(val[1]);

    /* Set the recognizer, aka program */
    pattern->program = (char *)malloc(strlen(val[0]) + 1);
    bzero(pattern->program, strlen(val[0]) + 1);
    strncpy(pattern->program, val[0], strlen(val[0]));

    OUTPUT_VERBOSE((7, "         %s (%d)", _GREEN(pattern->program),
                    pattern->id));

    /* Add this pattern to the transformation list */
    t = (transformation_t *)transformation;
    perfexpert_list_append((perfexpert_list_t *)&(t->patterns),
                           (perfexpert_list_item_t *)pattern);

    return PERFEXPERT_SUCCESS;
}

/* apply_recommendations */
static int apply_recommendations(fragment_t *fragment) {
    recommendation_t *recommendation;
    int rc = PERFEXPERT_NO_TRANS;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Applying recommendations")));

    /* For each recommendation in this fragment... */
    recommendation = (recommendation_t *)perfexpert_list_get_first(
        &(fragment->recommendations));
    while ((perfexpert_list_item_t *)recommendation !=
        &(fragment->recommendations.sentinel)) {

        /* Skip if other recommendation has already been applied */
        if (PERFEXPERT_SUCCESS == rc) {
            OUTPUT_VERBOSE((8, "   [%s ] [%d]", _MAGENTA("SKIP"),
                            recommendation->id));
            goto move_on;
        }

        /* Apply transformations */
        switch (apply_transformations(fragment, recommendation)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d]", _YELLOW("ERROR"),
                                recommendation->id));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d]", _RED("FAIL"),
                                recommendation->id));
                break;

            case PERFEXPERT_NO_TRANS:
                OUTPUT_VERBOSE((8, "   [%s ] [%d]", _MAGENTA("SKIP"),
                                recommendation->id));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((8, "   [ %s  ] [%d]", _BOLDGREEN("OK"),
                                recommendation->id));
                rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }

        /* Move to the next code recommendation */
        move_on:
        recommendation = (recommendation_t *)perfexpert_list_get_next(
            recommendation);
    }
    return rc;
}

/* apply_transformations */
static int apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation) {
    transformation_t *transformation;
    int rc = PERFEXPERT_FAILURE;

    /* Return when there is no transformations for this recommendation */
    if (0 == perfexpert_list_get_size(&(recommendation->transformations))) {
        return PERFEXPERT_NO_TRANS;
    }

    /* For each transformation for this recommendation... */
    transformation = (transformation_t *)perfexpert_list_get_first(
        &(recommendation->transformations));
    while ((perfexpert_list_item_t *)transformation !=
            &(recommendation->transformations.sentinel)) {

        /* Skip if other transformation has already been applied */
        if (PERFEXPERT_SUCCESS == rc) {
            OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                            transformation->id, transformation->program));
            goto move_on;
        }

        /* Apply patterns */
        switch (apply_patterns(fragment, recommendation, transformation)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _YELLOW("ERROR"),
                                transformation->id, transformation->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _RED("FAIL"),
                                transformation->id, transformation->program));
                goto move_on;

            case PERFEXPERT_NO_PAT:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                                transformation->id, transformation->program));
                goto move_on;

            case PERFEXPERT_SUCCESS:
            default:
                break;
        }

        /* Test transformation */
        switch (test_transformation(fragment, recommendation, transformation)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _YELLOW("ERROR"),
                                transformation->id, transformation->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _RED("FAIL"),
                                transformation->id, transformation->program));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((8, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                                transformation->id, transformation->program));
                rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }

        /* Move to the next code transformation */
        move_on:
        transformation = (transformation_t *)perfexpert_list_get_next(
            transformation);
    }
    return rc;
}

/* apply_patterns */
static int apply_patterns(fragment_t *fragment,
    recommendation_t *recommendation, transformation_t *transformation) {
    pattern_t *pattern;
    int rc = PERFEXPERT_FAILURE;

    /* Return when there is no patterns for this transformation */
    if (0 == perfexpert_list_get_size(&(transformation->patterns))) {
        return PERFEXPERT_NO_PAT;
    }

    /* For each pattern required for this transfomation ... */
    pattern = (pattern_t *)perfexpert_list_get_first(
        &(transformation->patterns));
    while ((perfexpert_list_item_t *)pattern != 
        &(transformation->patterns.sentinel)) {

        /* Extract fragments, for that we have to open ROSE */
        if (PERFEXPERT_ERROR == open_rose(fragment->filename)) {
            OUTPUT(("%s", _RED("Error: starting Rose")));
            return PERFEXPERT_FAILURE;
        } else {
            /* Hey ROSE, here we go... */
            if (PERFEXPERT_ERROR == extract_fragment(fragment)) {
                OUTPUT(("%s (%s:%d)",
                        _ERROR("Error: extracting fragments for"),
                        fragment->filename, fragment->line_number));
            }
            /* Close Rose */
                if (PERFEXPERT_SUCCESS != close_rose()) {
                OUTPUT(("%s", _ERROR("Error: closing Rose")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Test patterns */
        switch (test_pattern(fragment, recommendation, transformation,
                             pattern)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s] [%d] (%s)", _YELLOW("ERROR"),
                                pattern->id, pattern->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "   [%s ] [%d] (%s)", _RED("FAIL"),
                                pattern->id, pattern->program));
                break;

            case PERFEXPERT_SUCCESS:
                 OUTPUT_VERBOSE((7, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                                pattern->id, pattern->program));
               rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }

        /* Move to the next pattern */
        pattern = (pattern_t *)perfexpert_list_get_next(pattern);
    }
    return rc;
}

/* test_transformation */
static int test_transformation(fragment_t *fragment,
    recommendation_t *recommendation, transformation_t *transformation) {

    switch (run_transformer(fragment, recommendation, transformation)) {
        case PERFEXPERT_FAILURE:
            OUTPUT_VERBOSE((8, "   [%s ] [%s] >> [%s]", _ERROR("FAIL"),
                            transformation->program, fragment->filename));
            return PERFEXPERT_FAILURE;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((8, "   [ %s  ] [%s] >> [%s]", _BOLDGREEN("OK"),
                            transformation->program, fragment->filename));
            return PERFEXPERT_SUCCESS;

        case PERFEXPERT_ERROR:
            OUTPUT_VERBOSE((8, "   [%s] [%s] >> [%s]", _BOLDYELLOW("ERROR"),
                            transformation->program, fragment->filename));
            return PERFEXPERT_ERROR;

        case PERFEXPERT_UNDEFINED:
        default:
            OUTPUT_VERBOSE((8, "   [%s] [%s] >> [%s]", _BOLDRED("UNDEF"),
                            transformation->program, fragment->filename));
            return PERFEXPERT_UNDEFINED;
    }
}

/* test_pattern */
static int test_pattern(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation, pattern_t *pattern) {
    perfexpert_list_t tests;
    test_t *test;
    int rc = PERFEXPERT_FAILURE;

    /* Considering the outer loops, we can have more than one test */
    perfexpert_list_construct(&tests);

    /* The main test */
    test = (test_t *)malloc(sizeof(test_t));
    if (NULL == test) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)test);
    perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
    test->program = pattern->program;
    test->file    = fragment->fragment_file;
    test->result  = &(fragment->pattern_test_result);

    /* It we're testing for a loop, check for the outer loop */
    if ((0 == strncmp("loop", fragment->code_type, 4)) &&
        (2 <= fragment->loop_depth) &&
        (NULL != fragment->outer_loop_fragment_file)) {

        /* The outer loop test */
        test = (test_t *)malloc(sizeof(test_t));
        if (NULL == test) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        perfexpert_list_item_construct((perfexpert_list_item_t *)test);
        perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
        test->program = pattern->program;
        test->file    = fragment->outer_loop_fragment_file;
        test->result  = &(fragment->pattern_outer_loop_test_result);

        /* And test for the outer outer loop too */
        if ((3 <= fragment->loop_depth) &&
            (NULL != fragment->outer_outer_loop_fragment_file)) {

            /* The outer outer loop test */
            test = (test_t *)malloc(sizeof(test_t));
            if (NULL == test) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *)test);
            perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
            test->program = pattern->program;
            test->file    = fragment->outer_outer_loop_fragment_file;
            test->result  = &(fragment->pattern_outer_outer_loop_test_result);
        }
    }

    /* Run all the tests */
    test = (test_t *)perfexpert_list_get_first(&tests);
    while ((perfexpert_list_item_t *)test != &(tests.sentinel)) {
        switch (run_recognizer(test)) {
            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;


                break;

            case PERFEXPERT_ERROR:
            case PERFEXPERT_UNDEFINED:
            case PERFEXPERT_FAILURE:
            default:
                break;
        }
        test = (test_t *)perfexpert_list_get_next(
            (perfexpert_list_item_t *)test);
    }
    return rc;
}

/* apply_one */
static int run_transformer(fragment_t *fragment,
    recommendation_t *recommendation, transformation_t *transformation) {
    int  pid = 0;
    int  rc = PERFEXPERT_UNDEFINED;
    char temp_str[BUFFER_SIZE];
    char temp_str2[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    char argv[20][PARAM_SIZE];

    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/ct_%s", PERFEXPERT_BINDIR, transformation->program);

    /* Set the code transformer arguments. Ok, we have to define an
     * interface to code transformers. Here is a simple one. Each code
     * transformer will be called using the following arguments:
     *
     * -f FUNCTION  Function name were code bottleneck belongs to
     * -l LINE      Line number identified by HPCtoolkit/PerfExpert/etc...
     * -r FILE      File (maybe link) containing the transformation result
     * -s FILE      Source file
     * -w DIR       Use DIR as work directory
     */
    bzero(argv, PARAM_SIZE * 20);
    sprintf(argv[0], "ct_%s", transformation->program);
    sprintf(argv[1], "-f");
    sprintf(argv[2], "%s", fragment->function_name);
    sprintf(argv[3], "-l");
    sprintf(argv[4], "%d", fragment->line_number);
    sprintf(argv[5], "-r");
    sprintf(argv[6], "new_%s", fragment->filename);
    sprintf(argv[7], "-s");
    sprintf(argv[8], "%s", fragment->filename);
    sprintf(argv[9], "-w");
    sprintf(argv[10], "./");

    /* Set the command line */
    bzero(temp_str2, BUFFER_SIZE);
    sprintf(temp_str2,
            "%s %s %s %s %s %s %s %s %s %s %s %s",
            temp_str, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5],
            argv[6], argv[7], argv[8], argv[9], argv[10]);
    OUTPUT_VERBOSE((10, "      running %s", _CYAN(temp_str2)));

    /* Forking child */
    pid = fork();
    if (-1 == pid) {
        OUTPUT(("%s", _ERROR("Error: unable to fork")));
        return PERFEXPERT_ERROR;
    }

    if (0 == pid) {
        /* Child: Call the code transformer */
        // TODO: this is ridiculous. I have to change it to execp, but I'm too
        //       tired to do it now.
        execl(temp_str, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5],
              argv[6], argv[7], argv[8], argv[9], argv[10], NULL);

        OUTPUT(("child process failed to run, check if program exists"));
        exit(127);
    } else {
        /* Parent */
        wait(&rc);
        OUTPUT_VERBOSE((10, "      result  %s %d", _CYAN("return code"), rc >> 8));
    }

    /* Evaluating the result */
    switch (rc >> 8) {
        case 0: /* The transformation was possible */
            // PERFEXPERT_SUCCESS;
            break;

        case 255: /* The transformation was not possible */
            // PERFEXPERT_ERROR;
            break;

        case -1: /* Error during fork() or waitpid() */
            // PERFEXPERT_FAILURE;
            break;

        case 127: /* Execution failed */
            // PERFEXPERT_FAILURE;
            break;

        default: /* Not sure what happened */
            // PERFEXPERT_UNDEFINED;
            break;
    }
    return PERFEXPERT_SUCCESS;
}

/* test_one_pattern */
static int run_recognizer(test_t *test) {
    int  pipe1[2], pipe2[2];
    int  pid = 0;
    int  file = 0;
    int  r_bytes = 0, w_bytes = 0;
    int  rc = PERFEXPERT_UNDEFINED;
    char temp_str[BUFFER_SIZE];
    char temp_str2[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    #define PARENT_READ  pipe1[0]
    #define CHILD_WRITE  pipe1[1]
    #define CHILD_READ   pipe2[0]
    #define PARENT_WRITE pipe2[1]
    
    /* Creating pipes */
    if (-1 == pipe(pipe1)) {
        OUTPUT(("%s", _ERROR("Error: unable to create pipe1")));
        return PERFEXPERT_ERROR;
    }
    if (-1 == pipe(pipe2)) {
        OUTPUT(("%s", _ERROR("Error: unable to create pipe2")));
        return PERFEXPERT_ERROR;
    }
    
    /* Forking child */
    pid = fork();
    if (-1 == pid) {
        OUTPUT(("%s", _ERROR("Error: unable to fork")));
        return PERFEXPERT_ERROR;
    }

    if (0 == pid) {
        /* Child */
        bzero(temp_str, BUFFER_SIZE);
        bzero(temp_str2, BUFFER_SIZE);
        sprintf(temp_str, "%s/pr_%s", PERFEXPERT_BINDIR, test->program);
        sprintf(temp_str2, "pr_%s", test->program);
        OUTPUT_VERBOSE((10, "      running %s", _CYAN(temp_str)));
        
        close(PARENT_WRITE);
        close(PARENT_READ);

        if (-1 == dup2(CHILD_READ, STDIN_FILENO)) {
            OUTPUT(("%s", _ERROR("Error: unable to DUP STDIN")));
            return PERFEXPERT_ERROR;
        }
        if (-1 == dup2(CHILD_WRITE, STDOUT_FILENO)) {
            OUTPUT(("%s", _ERROR("Error: unable to DUP STDOUT")));
            return PERFEXPERT_ERROR;
        }

        execl(temp_str, temp_str2, NULL);
        
        OUTPUT(("child process failed to run, check if program exists"));
        exit(127);
    } else {
        /* Parent */
        close(CHILD_READ);
        close(CHILD_WRITE);
        
        /* Open input file and send it to the child process */
        if (-1 == (file = open(test->file, O_RDONLY))) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open fragment file"),
                    test->file));
            return PERFEXPERT_ERROR;
        } else {
            bzero(buffer, BUFFER_SIZE);
            while (0 != (r_bytes = read(file, buffer, BUFFER_SIZE))) {
                w_bytes = write(PARENT_WRITE, buffer, r_bytes);
                bzero(buffer, BUFFER_SIZE);
            }
            close(file);
            close(PARENT_WRITE);
        }
        
        /* Read child process' answer and write it to output file */
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s.%s.output", test->file, test->program);
        OUTPUT_VERBOSE((10, "      output  %s", _CYAN(temp_str)));

        if (-1 == (file = open(temp_str, O_CREAT|O_WRONLY, 0644))) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                    temp_str));
            return PERFEXPERT_ERROR;
        } else {
            bzero(buffer, BUFFER_SIZE);
            while (0 != (r_bytes = read(PARENT_READ, buffer, BUFFER_SIZE))) {
                w_bytes = write(file, buffer, r_bytes);
                bzero(buffer, BUFFER_SIZE);
            }
            close(file);
            close(PARENT_READ);
        }
        wait(&rc);
        OUTPUT_VERBOSE((10, "      result  %s %d", _CYAN("return code"),
                        rc >> 8));
    }

    /* Evaluating the result */
    switch (rc >> 8) {
        case -1: /* Error during fork() or waitpid() */
            OUTPUT_VERBOSE((7, "   [%s] [%s] >> [%s]", _BOLDYELLOW("ERROR"),
                            test->program, test->file));
            *test->result = PERFEXPERT_ERROR;
            break;

        case 0: /* The pattern matches */
            OUTPUT_VERBOSE((7, "   [ %s  ] [%s] >> [%s]", _BOLDGREEN("OK"),
                            test->program, test->file));
            *test->result = PERFEXPERT_SUCCESS;;
            break;

        case 255: /* The pattern doesn't match */
            OUTPUT_VERBOSE((7, "   [%s ] [%s] >> [%s]", _BOLDRED("FAIL"),
                            test->program, test->file));
            *test->result = PERFEXPERT_FAILURE;
            break;

        case 127: /* Execution failed */
            OUTPUT_VERBOSE((7, "   [%s ] [%s] >> [%s]", _BOLDRED("FAIL"),
                            test->program, test->file));
            *test->result = PERFEXPERT_FAILURE;
            break;

        default: /* Not sure what happened */
            OUTPUT_VERBOSE((7, "   [%s] [%s] >> [%s]", _MAGENTA("UNDEF"),
                            test->program, test->file));
            *test->result = PERFEXPERT_UNDEFINED;
            break;
    }
    return *test->result;
}

#ifdef __cplusplus
}
#endif

// EOF
