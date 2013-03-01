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
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#if HAVE_SQLITE3 == 1
/* Utility headers */
#include <sqlite3.h>
#endif

/* OptTran headers */
#include "config.h"
#include "ct.h"
#include "opttran_output.h"
#include "opttran_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    fragment_t *fragment;
    opttran_list_t *fragments;
    transformation_t *transformation;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose          = 0,      // int
        .verbose_level    = 0,      // int
        .use_stdin        = 0,      // int
        .use_stdout       = 1,      // int
        .inputfile        = NULL,   // char *
        .outputfile       = NULL,   // char *
        .outputfile_FP    = stdout, // FILE *
        .opttrandir       = NULL,   // char *
#if HAVE_SQLITE3 == 1
        .opttran_pid      = (unsigned long long int)getpid(), // int
#endif
        .colorful         = 0       // int
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

    /* Parse command-line parameters */
    if (OPTTRAN_SUCCESS != parse_cli_params(argc, argv)) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        exit(OPTTRAN_ERROR);
    }

    /* Create the list of fragments */
    fragments = (opttran_list_t *)malloc(sizeof(opttran_list_t));
    if (NULL == fragments) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    opttran_list_construct(fragments);

    /* Parse input parameters */
    if (1 == globals.use_stdin) {
        if (OPTTRAN_SUCCESS != parse_transformation_params(fragments, stdin)) {
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
                if (OPTTRAN_SUCCESS != parse_transformation_params(fragments,
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

    /* Apply transformation */

    /* Output results */
    
    /* Free memory */
    while (OPTTRAN_FALSE == opttran_list_is_empty(fragments)) {
        fragment = (fragment_t *)opttran_list_get_first(fragments);
        opttran_list_remove_item(fragments, (opttran_list_item_t *)fragment);
        while (OPTTRAN_FALSE == opttran_list_is_empty(&(fragment->transformations))) {
            transformation = (transformation_t *)opttran_list_get_first(&(fragment->transformations));
            opttran_list_remove_item(&(fragment->transformations),
                                     (opttran_list_item_t *)transformation);
            free(transformation->program);
            free(transformation);
        }
        free(fragment);
    }
    opttran_list_destruct(fragments);
    free(fragments);
    free(globals.dbfile);

    return OPTTRAN_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OPTTRAN_OUTPUT_VERBOSE((10, "printing help"));

    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: opttran_ct -i|-f file [-o file] [-vch] [-l level] [-a dir]");
#if HAVE_SQLITE3 == 1
    printf(" [-d database] [-p pid]");
#endif
    printf("\n");
    printf("  -i --stdin           Use STDIN as input for patterns\n");
    printf("  -f --inputfile       Use 'file' as input for patterns\n");
    printf("  -o --outputfile      Use 'file' as output (default stdout)\n");
    printf("  -a --opttran         Create OptTran (automatic performance optimization) files\n");
    printf("                       into 'dir' directory (default: create no OptTran files).\n");
    printf("                       This argument overwrites -o (no output on STDOUT, except\n");
    printf("                       for verbose messages)\n");
#if HAVE_SQLITE3 == 1
    printf("  -d --database        Select the recommendation database file\n");
    printf("                       (default: %s/%s)\n", OPTTRAN_VARDIR, RECOMMENDATION_DB);
    printf("  -p --opttranid       Use 'pid' to log on DB consecutive calls to Recommender\n");
#endif
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
#if HAVE_SQLITE3 == 1
        parameter = getopt_long(argc, argv, "a:cvhif:l:o:p:t", long_options,
                                &option_index);
#else
        parameter = getopt_long(argc, argv, "a:cvhif:l:o:t", long_options,
                                &option_index);
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
#if HAVE_SQLITE3 == 1
            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'd' set [%s]",
                                        globals.dbfile));
                break;

            /* Specify OptTran PID */
            case 'p':
                globals.opttran_pid = strtoull(optarg, (char **)NULL, 10);
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'p' set [%llu]",
                                        globals.opttran_pid));
                break;
#endif
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

            /* Use opttran? */
            case 'a':
                globals.use_opttran = 1;
                globals.use_stdout = 0;
                globals.opttrandir = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'a' set [%s]",
                                        globals.opttrandir));
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
    OPTTRAN_OUTPUT_VERBOSE((10, "   Verbose:          %s",
                            globals.verbose ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Verbose level:    %d",
                            globals.verbose_level));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Colorful verbose? %s",
                            globals.colorful ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use STDOUT?       %s",
                            globals.use_stdout ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use STDIN?        %s",
                            globals.use_stdin ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Input file:       %s",
                            globals.inputfile ? globals.inputfile : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Output file:      %s",
                            globals.outputfile ? globals.outputfile : "(null)"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Use OPTTRAN?      %s",
                            globals.use_opttran ? "yes" : "no"));
    OPTTRAN_OUTPUT_VERBOSE((10, "   OPTTRAN PID:      %llu",
                            globals.opttran_pid));
    OPTTRAN_OUTPUT_VERBOSE((10, "   Database file:    %s",
                            globals.dbfile ? globals.dbfile : "(null)"));

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

/* parse_transformation_params */
static int parse_transformation_params(opttran_list_t *fragments_p,
                                       FILE *inputfile_p) {
    fragment_t *fragment;
    transformation_t *transformation;
    char buffer[BUFFER_SIZE];
    int  input_line = 0;

    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing fragments file")));

    /* Which INPUT we are using? (just a double check) */
    if ((NULL == inputfile_p) && (globals.use_stdin)) {
        inputfile_p = stdin;
    }
    if (globals.use_stdin) {
        OPTTRAN_OUTPUT_VERBOSE((3, "using STDIN as input for fragments"));
    } else {
        OPTTRAN_OUTPUT_VERBOSE((3, "using (%s) as input for fragments",
                                globals.inputfile));
    }

    /* For each line in the INPUT file... */
    OPTTRAN_OUTPUT_VERBOSE((7, "--- parsing input file"));

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, inputfile_p)) {
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

            OPTTRAN_OUTPUT_VERBOSE((5, "(%d) --- %s", input_line,
                                    _GREEN("new fragment found")));

            /* Create a list item for this code fragment */
            fragment = (fragment_t *)malloc(sizeof(fragment_t));
            if (NULL == fragment) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *)fragment);

            /* Initialize some elements on 'fragment' */
            fragment->fragment_file = NULL;
            opttran_list_construct((opttran_list_t *)&(fragment->transformations));

            /* Add this item to 'fragments_p' */
            opttran_list_append(fragments_p, (opttran_list_item_t *)fragment);

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
         * 'patterns'.
         */

        /* Code param: recommender.code_fragment */
        if (0 == strncmp("recommender.code_fragment", node->key, 25)) {
            fragment->fragment_file = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->fragment_file) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(fragment->fragment_file, strlen(node->value) + 1);
            strcpy(fragment->fragment_file, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) %s      [%s]", input_line,
                                    _MAGENTA("fragment file:"),
                                    fragment->fragment_file));
            free(node);
            continue;
        }

        /* Code param: pr.transformation */
        if (0 == strncmp("pr.transformation", node->key, 17)) {
            transformation = (transformation_t *)malloc(sizeof(transformation_t));
            if (NULL == transformation) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *)transformation);
            opttran_list_append((opttran_list_t *)&(fragment->transformations),
                                (opttran_list_item_t *)transformation);

            transformation->program = (char *)malloc(strlen(node->value) + 1);
            if (NULL == transformation->program) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }

            bzero(transformation->program, strlen(node->value) + 1);
            strcpy(transformation->program, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  \\- %s [%s]", input_line,
                                    _YELLOW("transformation:"),
                                    transformation->program));
            free(node);
            continue;
        }

        /* Should never reach this point, only if there is garbage in the input
         * file.
         */
        OPTTRAN_OUTPUT_VERBOSE((4, "(%d) %s (%s = %s)", input_line,
                                _RED("ignored line"), node->key, node->value));
        free(node);
    }

    /* print a summary of 'fragments' */
    OPTTRAN_OUTPUT_VERBOSE((4, "%d %s", opttran_list_get_size(fragments_p),
                            _GREEN("code fragment(s) found")));

    fragment = (fragment_t *)opttran_list_get_first(fragments_p);
    while ((opttran_list_item_t *)fragment != &(fragments_p->sentinel)) {
        OPTTRAN_OUTPUT_VERBOSE((4, "   %s", fragment->fragment_file));
        fragment = (fragment_t *)opttran_list_get_next(fragment);
    }

    OPTTRAN_OUTPUT_VERBOSE((4, "==="));

    return OPTTRAN_SUCCESS;
}

#if HAVE_SQLITE3 == 1
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
#endif

#ifdef __cplusplus
}
#endif

// EOF
