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
#include <unistd.h>
#include <getopt.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_database.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    perfexpert_list_t *segments;
    segment_t *segment;
    function_t *function;
    int rc = PERFEXPERT_NO_REC;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level    = 0,             // int
        .inputfile        = NULL,          // char *
        .inputfile_FP     = stdin,         // FILE *
        .outputfile       = NULL,          // char *
        .outputfile_FP    = stdout,        // FILE *
        .dbfile           = NULL,          // char *
        .workdir          = NULL,          // char *
        .perfexpert_pid   = (unsigned long long int)getpid(), // int
        .colorful         = 0,             // int
        .metrics_file     = NULL,          // char *
        .use_temp_metrics = 0,             // int
        .metrics_table    = METRICS_TABLE, // char *
        .rec_count        = 3              // int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create the list of code bottlenecks */
    segments = (perfexpert_list_t *)malloc(sizeof(perfexpert_list_t));
    if (NULL == segments) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_construct(segments);
    
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
        OUTPUT_VERBOSE((7, "   %s (%s)",
                        _YELLOW("printing recommendations to file"),
                        globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                    globals.outputfile));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }
    } else {
        OUTPUT_VERBOSE((7, "   printing recommendations to STDOUT"));
    }

    /* Connect to database */
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
                                                          globals.dbfile)) {
        OUTPUT(("%s", _ERROR("Error: connecting to database")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }
    
    /* Parse metrics file if 'm' is defined, this will create a temp table */
    if (1 == globals.use_temp_metrics) {
        globals.metrics_table = (char *)malloc(strlen("metrics_") + 6);
        sprintf(globals.metrics_table, "metrics_%d", (int)getpid());

        if (PERFEXPERT_SUCCESS != parse_metrics_file()) {
            OUTPUT(("%s", _ERROR("Error: parsing metrics file")));
            rc = PERFEXPERT_ERROR;
            goto cleanup;
        }
    }
    
    /* Parse input parameters */
    if (PERFEXPERT_SUCCESS != parse_segment_params(segments)) {
        OUTPUT(("%s", _ERROR("Error: parsing input params")));
        rc = PERFEXPERT_ERROR;
        goto cleanup;
    }

    /* Select recommendations for each code bottleneck */
    segment = (segment_t *)perfexpert_list_get_first(segments);
    while ((perfexpert_list_item_t *)segment != &(segments->sentinel)) {
        OUTPUT_VERBOSE((4, "%s (%s:%d)",
                        _YELLOW("selecting recommendation for"),
                        segment->filename, segment->line_number));

        /* Query DB for recommendations */
        switch (select_recommendations(segment)) {
            case PERFEXPERT_NO_REC:
                OUTPUT(("%s", _GREEN("Sorry, we have no recommendations")));
                /* Move to the next code bottleneck */
                segment = (segment_t *)perfexpert_list_get_next(segment);
                continue;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            default: 
                OUTPUT(("%s", _ERROR("Error: selecting recommendations")));
                rc = PERFEXPERT_ERROR;
                goto cleanup;
        }
        /* Move to the next code bottleneck */
        segment = (segment_t *)perfexpert_list_get_next(segment);
    }

    cleanup:
    /* Close input file */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    perfexpert_database_disconnect(globals.db);

    /* Free memory */

    return rc;
}

/* show_help */
static void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));
    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: recommender -f file [-l level] [-o file] [-d database] [-m file] [-nvch]\n");
    #if HAVE_ROSE == 1
    printf("                   [-a workdir]");
    #endif
    printf("\n");
    printf("  -f --inputfile       Use 'file' as input for performance measurements\n");
    printf("  -o --outputfile      Use 'file' as output for recommendations (default: STDOUT)\n");
    printf("                       if the file exists its content will be overwritten\n");
    printf("  -d --database        Select the recommendation database file\n");
    printf("                       (default: %s/%s)\n", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
    printf("  -m --metricsfile     Use 'file' to define metrics different from the default\n");
    printf("  -n --newmetrics      Do not use the system metrics table. A temporary table\n");
    printf("                       will be created using the default metrics file:\n");
    printf("                       %s/%s\n", PERFEXPERT_ETCDIR, METRICS_FILE);
    printf("  -r --recommendations Number of recommendation to show\n");
    #if HAVE_ROSE == 1
    printf("  -a --automatic       Use automatic performance optimization and create files\n");
    printf("                       into 'workdir' directory (default: off).\n");
    #endif
    printf("  -v --verbose         Enable verbose mode using default verbose level (5)\n");
    printf("  -l --verbose_level   Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful        Enable colors on verbose mode, no weird characters will\n");
    printf("                       appear on output files\n");
    printf("  -h --help            Show this message\n");
}

/* parse_env_vars */
static int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL")) {
        if ((0 >= atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL"))) ||
            (10 < atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL")))) {
            OUTPUT(("%s (%d)", _ERROR("ENV Error: invalid debug level"),
                    atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL"))));
            show_help();
            return PERFEXPERT_ERROR;
        }
        globals.verbose_level = atoi(getenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_INPUT_FILE")) {
        globals.inputfile = getenv("PERFEXPERT_RECOMMENDER_INPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: inputfile=%s", globals.inputfile));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE")) {
        globals.outputfile = getenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE");
        OUTPUT_VERBOSE((5, "ENV: outputfile=%s", globals.outputfile));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE")) {
        globals.dbfile =  getenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE");
        OUTPUT_VERBOSE((5, "ENV: dbfile=%s", globals.dbfile));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE")) {
        globals.use_temp_metrics = 1;
        globals.metrics_file = getenv("PERFEXPERT_RECOMMENDER_METRICS_FILE");
        OUTPUT_VERBOSE((5, "ENV: metrics_file=%s", globals.metrics_file));
    }

    if ((NULL != getenv("PERFEXPERT_RECOMMENDER_REC_COUNT")) &&
        (0 <= atoi(getenv("PERFEXPERT_RECOMMENDER_REC_COUNT")))) {
        globals.rec_count = atoi(getenv("PERFEXPERT_RECOMMENDER_REC_COUNT"));
        OUTPUT_VERBOSE((5, "ENV: rec_count=%d", globals.rec_count));
    }

    if (NULL != getenv("PERFEXPERT_RECOMMENDER_WORKDIR")) {
        globals.workdir = getenv("PERFEXPERT_RECOMMENDER_WORKDIR");
        OUTPUT_VERBOSE((5, "ENV: workdir=%s", globals.workdir));
    }

    if ((NULL != getenv("PERFEXPERT_RECOMMENDER_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_RECOMMENDER_COLORFUL")))) {
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
    #if HAVE_ROSE == 1
        parameter = getopt_long(argc, argv, "a:cd:f:hl:m:no:p:r:v",
                                long_options, &option_index);
    #else
        parameter = getopt_long(argc, argv, "cd:f:hl:m:no:p:r:v",
                                long_options, &option_index);
    #endif
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
                globals.colorful = 1;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();
                exit(PERFEXPERT_SUCCESS);

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

            #if HAVE_ROSE == 1
            /* Use automatic performance optimization? */
            case 'a':
                globals.workdir = optarg;
                OUTPUT_VERBOSE((10, "option 'a' set [%s]", globals.workdir));
                break;
            #endif

            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OUTPUT_VERBOSE((10, "option 'd' set [%s]", globals.dbfile));
                break;
                
            /* Use temporary metrics table */
            case 'n':
                globals.use_temp_metrics = 1;
                OUTPUT_VERBOSE((10, "option 'n' set"));
                break;
                
            /* Specify new metrics */
            case 'm':
                globals.use_temp_metrics = 1;
                globals.metrics_file = optarg;
                OUTPUT_VERBOSE((10, "option 'm' set [%s]",
                                globals.metrics_file));
                break;
                
            /* Specify PerfExpert PID */
            case 'p':
                globals.perfexpert_pid = strtoull(optarg, (char **)NULL, 10);
                OUTPUT_VERBOSE((10, "option 'p' set [%llu]",
                                globals.perfexpert_pid));
                break;
                
            /* Number of recommendation to output */
            case 'r':
                globals.rec_count = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'r' set [%d]", globals.rec_count));
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
    OUTPUT_VERBOSE((10, "   Verbose level:     %d", globals.verbose_level));
    OUTPUT_VERBOSE((10, "   Colorful verbose?  %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   PerfExpert PID:    %llu",
                    globals.perfexpert_pid));
    OUTPUT_VERBOSE((10, "   Work directory:    %s", globals.workdir));
    OUTPUT_VERBOSE((10, "   Input file:        %s", globals.inputfile));
    OUTPUT_VERBOSE((10, "   Output file:       %s", globals.outputfile));
    OUTPUT_VERBOSE((10, "   Database file:     %s", globals.dbfile));
    OUTPUT_VERBOSE((10, "   Metrics file:      %s", globals.metrics_file));
    OUTPUT_VERBOSE((10, "   Temporary metrics: %s",
                    globals.use_temp_metrics ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Metrics table:     %s", globals.metrics_table));
    OUTPUT_VERBOSE((10, "   Recommendations:   %d", globals.rec_count));

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

/* parse_segment_params */
static int parse_segment_params(perfexpert_list_t *segments_p) {
    segment_t *item;
    int  input_line = 0;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    int  rowid = 0;
    char *temp_str = NULL;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing measurements")));
    
    /* Which INPUT we are using? (just a double check) */
    if (NULL == globals.inputfile) {
        OUTPUT_VERBOSE((3, "using STDIN as input for perf measurements"));
    } else {
        OUTPUT_VERBOSE((3, "using (%s) as input for perf measurements",
                        globals.inputfile));
    }

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "BEGIN TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
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

        /* Is this line a new code bottleneck specification? */
        if (0 == strncmp("%", buffer, 1)) {
            char temp_str[BUFFER_SIZE];
            
            OUTPUT_VERBOSE((5, "(%d) --- %s", input_line,
                            _GREEN("new bottleneck found")));

            /* Create a list item for this code bottleneck */
            item = (segment_t *)malloc(sizeof(segment_t));
            if (NULL == item) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *) item);
            
            /* Initialize some elements on segment */
            item->filename = NULL;
            item->line_number = 0;
            item->type = NULL;
            item->extra_info = NULL;
            item->section_info = NULL;
            item->loop_depth = 0;
            item->representativeness = 0;
            item->rowid = 0;
            item->function_name  = NULL;

            /* Add this item to 'segments' */
            perfexpert_list_append(segments_p, (perfexpert_list_item_t *) item);

            /* Create the SQL statement for this new segment */
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                    "INSERT INTO %s (code_filename) VALUES ('new_code-%d'); ",
                    globals.metrics_table, (int)getpid());
            bzero(sql, BUFFER_SIZE);
            strcat(sql, temp_str);
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                    "SELECT id FROM %s WHERE code_filename = 'new_code-%d';",
                    globals.metrics_table, (int)getpid());
            strcat(sql, temp_str);

            OUTPUT_VERBOSE((5, "        SQL: %s", _CYAN(sql)));
            
            /* Insert new code fragment into metrics database, retrieve id */
            if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                                          perfexpert_database_get_int,
                                          (void *)&rowid, &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                return PERFEXPERT_ERROR;
            } else {
                OUTPUT_VERBOSE((5, "        ID: %d", rowid));
                /* Store the rowid on the segment structure */
                item->rowid = rowid;
            }

            /* Set PerfExpert PID for the new segment */
            bzero(sql, BUFFER_SIZE);
            sprintf(sql, "UPDATE %s SET pid=%llu WHERE id=%d;",
                    globals.metrics_table, globals.perfexpert_pid, rowid);
            if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                          &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                return PERFEXPERT_ERROR;
            }

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

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments' (only code.* parameters) other parameter should fit into
         * the SQL DB.
         */

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            item->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->filename) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->filename, strlen(node->value) + 1);
            strcpy(item->filename, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s     [%s]", input_line,
                            _MAGENTA("filename:"), item->filename));
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            item->line_number = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) %s  [%d]", input_line,
                            _MAGENTA("line number:"), item->line_number));
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            item->type = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->type) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->type, strlen(node->value) + 1);
            strcpy(item->type, node->value);
            OUTPUT_VERBOSE((10, "(%d) type:         [%s]",
                            input_line, item->type));
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            item->extra_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->extra_info) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->extra_info, strlen(node->value) + 1);
            strcpy(item->extra_info, node->value);
            OUTPUT_VERBOSE((10, "(%d) extra info:   [%s]", input_line,
                            item->extra_info));
        }
        /* Code param: code.representativeness */
        if (0 == strncmp("code.representativeness", node->key, 23)) {
            item->representativeness = atof(node->value);
            OUTPUT_VERBOSE((10, "(%d) representativeness: [%f], ",
                            input_line, item->representativeness));
            free(node);
            continue;
        }
        /* Code param: code.section_info */
        if (0 == strncmp("code.section_info", node->key, 17)) {
            item->section_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->section_info) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->section_info, strlen(node->value) + 1);
            strcpy(item->section_info, node->value);
            OUTPUT_VERBOSE((10, "(%d) section info: [%s]", input_line,
                            item->section_info));
            free(node);
            continue;
        }
        /* Code param: code.function_name */
        if (0 == strncmp("code.function_name", node->key, 18)) {
            /* Remove everyting after the '.' (for OMP functions) */
            temp_str = node->value;
            strsep(&temp_str, ".");

            item->function_name = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->function_name) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->function_name, strlen(node->value) + 1);
            strcpy(item->function_name, node->value);
            OUTPUT_VERBOSE((10, "(%d) function name: [%s]", input_line,
                            item->function_name));
            free(node);
            continue;
        }
        /* Code param: perfexpert.loop_depth */
        if (0 == strncmp("perfexpert.loop-depth", node->key, 21)) {
            item->loop_depth = atof(node->value);
            OUTPUT_VERBOSE((10, "(%d) loop depth: [%f]", input_line,
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
            OUTPUT_VERBOSE((4, "(%d) %s (%s = %s)", input_line,
                            _RED("ignored line"), node->key, node->value));
            sqlite3_free(error_msg);
        } else {
            OUTPUT_VERBOSE((10, "(%d) %s", input_line, sql));
        }
        free(node);
    }

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "END TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* print a summary of 'segments' */
    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(segments_p),
                    _GREEN("code segment(s) found")));

    item = (segment_t *)perfexpert_list_get_first(segments_p);
    while ((perfexpert_list_item_t *)item != &(segments_p->sentinel)) {
        OUTPUT_VERBOSE((4, "   %s:%d", item->filename, item->line_number));
        item = (segment_t *)perfexpert_list_get_next(item);
    }
    
    return PERFEXPERT_SUCCESS;
}

/* select_recommendations */
static int select_recommendations(segment_t *segment) {
    char *error_msg = NULL;
    function_t *function;
    char sql[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE];
    double weight = 0;
    int rc;

    perfexpert_list_construct((perfexpert_list_t *) &(segment->functions));
    
    OUTPUT_VERBOSE((7, "=== %s", _BLUE("Querying DB")));

    if (NULL != globals.workdir) {
        fprintf(globals.outputfile_FP, "%% recommendation for %s:%d\n",
                segment->filename, segment->line_number);
        fprintf(globals.outputfile_FP, "code.filename=%s\n",
                segment->filename);
        fprintf(globals.outputfile_FP, "code.line_number=%d\n",
                segment->line_number);
        fprintf(globals.outputfile_FP, "code.type=%s\n", segment->type);
        fprintf(globals.outputfile_FP, "code.function_name=%s\n",
                segment->function_name);
        fprintf(globals.outputfile_FP, "code.loop_depth=%1.0lf\n",
                segment->loop_depth);
        fprintf(globals.outputfile_FP, "code.rowid=%d\n", segment->rowid);
    } else {
        fprintf(globals.outputfile_FP,
                "#--------------------------------------------------\n");
        fprintf(globals.outputfile_FP, "# Recommendations for %s:%d\n",
                segment->filename, segment->line_number);
        fprintf(globals.outputfile_FP,
                "#--------------------------------------------------\n");
    }
    
    /* Select all functions, accumulate them */
    OUTPUT_VERBOSE((8, "%s [%s:%d, ROWID: %d]",
                    _YELLOW("looking for functions to segment"),
                    segment->filename, segment->line_number, segment->rowid));
    
    if (SQLITE_OK != sqlite3_exec(globals.db,
                                  "SELECT id, desc, statement FROM function;",
                                  accumulate_functions,
                                  (void *)&(segment->functions),
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* Execute all functions, accumulate recommendations */
    OUTPUT_VERBOSE((8, "%d %s", perfexpert_list_get_size(&(segment->functions)),
                    _GREEN("functions found")));

    /* Create a temporary table to store possible recommendations */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "CREATE TEMP TABLE recommendation_%d_%d (function_id INTEGER,",
            (int)getpid(), segment->rowid);
    strcat(sql, " recommendation_id INTEGER, score FLOAT, weigth FLOAT);");

    OUTPUT_VERBOSE((10, "%s",
                    _YELLOW("creating temporary table of recommendations")));
    OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));
    
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* For each function, execute it! */
    function = (function_t *)perfexpert_list_get_first(&(segment->functions));
    while ((perfexpert_list_item_t *)function != &(segment->functions.sentinel)) {
        sqlite3_stmt *statement;
        
        OUTPUT_VERBOSE((8, "%s (ID: %d, %s) [%d bytes]",
                        _YELLOW("running function"), function->id,
                        function->desc, strlen(function->statement)));

        /* Prepare the SQL statement */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, function->statement,
                                            BUFFER_SIZE, &statement, NULL)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Bind ROWID */
        if (SQLITE_OK != sqlite3_bind_int(statement,
                                          sqlite3_bind_parameter_index(statement,
                                                                       "@RID"),
                                          segment->rowid)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Bind loop depth */
        if (SQLITE_OK != sqlite3_bind_double(statement,
                                             sqlite3_bind_parameter_index(statement,
                                                                          "@LPD"),
                                             segment->loop_depth)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Run que query */
        OUTPUT_VERBOSE((10, "   %s", _GREEN("list of recommendations: ")));
        do {
            rc = sqlite3_step(statement);

            /* It is possible that this function does not return results */
            if (SQLITE_DONE == rc) {
                OUTPUT_VERBOSE((10,
                                "      end of recommendations for function (%d)",
                                function->id));
                continue;
            } else if (SQLITE_ROW == rc) {
                /* check if there are at least two columns in the SQL result and
                 * their types are SQLITE_INTEGER and SQLITE_FLOAT respectivelly
                 */
                if (SQLITE_INTEGER != sqlite3_column_type(statement, 0)) {
                    OUTPUT(("%s", _ERROR("1st column is not an integer.")));
                    continue;
                }
                if (SQLITE_FLOAT != sqlite3_column_type(statement, 1)) {
                    OUTPUT(("%s", _ERROR("2nd column is not a double.")));
                    continue;
                }
                
                /* Consider only the results where the score is positive */
                if (0 < sqlite3_column_double(statement, 1)) {
                    /* Find the weigth of this recommendation */
                    weight = 0;
                    bzero(sql, BUFFER_SIZE);
                    strcat(sql,
                           "SELECT weight FROM recommendation_function WHERE");
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str,
                            " id_recommendation=%d AND id_function=%d;",
                            sqlite3_column_int(statement, 0), function->id);
                    strcat(sql, temp_str);
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                                                  perfexpert_database_get_double,
                                                  (void *)&weight,
                                                  &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        return PERFEXPERT_ERROR;
                    }
                    
                    /* Insert recommendation into the temporary table */
                    bzero(sql, BUFFER_SIZE);
                    sprintf(sql, "INSERT INTO recommendation_%d_%d ",
                            (int)getpid(), segment->rowid);
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str, "VALUES (%d, %d, %f, %f);", function->id,
                            sqlite3_column_int(statement, 0),
                            sqlite3_column_double(statement, 1), weight);
                    strcat(sql, temp_str);
                    
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                                  &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        return PERFEXPERT_ERROR;
                    }
                    
                    OUTPUT_VERBOSE((10, "      FunctionID=%d, RecommendationID=%d, Score=%f, Weight=%f",
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
            return PERFEXPERT_ERROR;
        }
        
        /* SQLite3 cleanup */
        if (SQLITE_OK != sqlite3_reset(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        if (SQLITE_OK != sqlite3_finalize(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        
        /* Move to the next code bottleneck */
        function = (function_t *)perfexpert_list_get_next(function);
    }

    /* Calculate the normalized rank of recommendations, should use weights? */
    // TODO: someday, I will add the weighting system here (Leo)

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, BUFFER_SIZE);
    strcat(sql, "SELECT r.desc AS desc, r.reason AS reason, r.id AS id, ");
    strcat(sql, "r.example AS example FROM recommendation AS r INNER JOIN ");
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "recommendation_%d_%d AS m ON r.id = m.recommendation_id",
            (int)getpid(), segment->rowid);
    strcat(sql, temp_str);
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, " ORDER BY m.score DESC LIMIT %d;", globals.rec_count);
    strcat(sql, temp_str);
    
    OUTPUT_VERBOSE((10, "%s (top %d)",
                    _YELLOW("selecting top-N recommendations"),
                    globals.rec_count));
    OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));

    rc = PERFEXPERT_NO_REC;

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_recommendations,
                                  (void *)&(rc), &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    return rc;
}

/* parse_metrics_file */
static int parse_metrics_file(void) {
    FILE *metrics_FP;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;

    if (NULL == globals.metrics_file) {
        globals.metrics_file = (char *)malloc(strlen(METRICS_FILE) +
                                              strlen(PERFEXPERT_ETCDIR) + 2);
        if (NULL == globals.metrics_file) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(globals.metrics_file, strlen(METRICS_FILE) +
              strlen(PERFEXPERT_ETCDIR) + 2);
        sprintf(globals.metrics_file, "%s/%s", PERFEXPERT_ETCDIR, METRICS_FILE);
    }

    OUTPUT_VERBOSE((7, "=== %s (%s)", _BLUE("Reading metrics file"),
                    globals.metrics_file));

    if (NULL == (metrics_FP = fopen(globals.metrics_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open metrics file"),
                globals.metrics_file));
        return PERFEXPERT_ERROR;
    } else {
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "CREATE TEMP TABLE %s ( ", globals.metrics_table);
        strcat(sql, "id INTEGER PRIMARY KEY, code_filename CHAR( 1024 ), ");
        strcat(sql, "code_line_number INTEGER, code_type CHAR( 128 ), ");
        strcat(sql, "code_extra_info CHAR( 1024 ), ");

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
            strcat(sql, buffer);
            strcat(sql, " FLOAT, ");
        }
        sql[strlen(sql)-2] = '\0'; // remove the last ',' and '\n'
        strcat(sql, ");");
        OUTPUT_VERBOSE((10, "metrics SQL: %s", _CYAN(sql)));

        /* Create metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                      &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            fprintf(stderr, "SQL clause: %s\n", sql);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        OUTPUT(("using temporary metric table (%s)", globals.metrics_table));
    }
    return PERFEXPERT_SUCCESS;
}

/* output_recommendations */
static int output_recommendations(void *var, int count, char **val,
    char **names) {
    int *rc = (int *)var;

    OUTPUT_VERBOSE((7, "%s", _GREEN("new recommendation found")));
    
    if (NULL == globals.workdir) {
        /* Pretty print for the user */
        fprintf(globals.outputfile_FP, "#\n# Here is a possible recommendation");
        fprintf(globals.outputfile_FP, " for this code segment\n#\n");
        fprintf(globals.outputfile_FP, "ID: %s\n", val[2]);
        fprintf(globals.outputfile_FP, "Description: %s\n", val[0]);
        fprintf(globals.outputfile_FP, "Reason: %s\n", val[1]);
        fprintf(globals.outputfile_FP, "Code example:\n%s\n", val[3]);
    } else {
        /* PerfExpert output */
        fprintf(globals.outputfile_FP, "recommender.recommendation_id=%s\n",
                val[2]);
    }

    *rc = PERFEXPERT_SUCCESS;
    return PERFEXPERT_SUCCESS;
}

/* accumulate_functions */
static int accumulate_functions(void *functions, int count, char **val,
    char **names) {
    function_t *function;
    
    /* Copy SQL query result into functions list */
    function = (function_t *)malloc(sizeof(function_t));
    if (NULL == function) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        goto cleanup;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)function);

    function->id = atoi(val[0]);
    
    function->desc = (char *)malloc(strlen(val[1]) + 1);
    if (NULL == function->desc) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        goto cleanup;
    }
    bzero(function->desc, strlen(val[1]) + 1);
    strncpy(function->desc, val[1], strlen(val[1]));
    
    bzero(function->statement, BUFFER_SIZE);
    strncpy(function->statement, val[2], strlen(val[2]));

    perfexpert_list_append((perfexpert_list_t *)functions,
                           (perfexpert_list_item_t *)function);

    OUTPUT_VERBOSE((10, "%s (ID: %d, %s) [%d bytes]", _YELLOW("function found"),
                    function->id, function->desc, strlen(function->statement)));

    return PERFEXPERT_SUCCESS;

    cleanup:
    /* Close input file */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }
    return PERFEXPERT_ERROR;
}

#ifdef __cplusplus
}
#endif

// EOF
