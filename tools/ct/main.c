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
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <getopt.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <fcntl.h>
//#include <inttypes.h>

#if HAVE_SQLITE3
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
#if HAVE_SQLITE3
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


    /* Parse input parameters */
    if (1 == globals.use_stdin) {
        if (OPTTRAN_SUCCESS != parse_fragment_params(fragments, stdin)) {
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
                if (OPTTRAN_SUCCESS != parse_fragment_params(fragments,
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

    /* Free memory */
    free(globals.dbfile);

    return OPTTRAN_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OPTTRAN_OUTPUT_VERBOSE((10, "printing help"));

    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: opttran_ct -i|-f file [-o file] [-vch] [-l level] [-a dir]");
#if HAVE_SQLITE3
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
#if HAVE_SQLITE3
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
#if HAVE_SQLITE3
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
#if HAVE_SQLITE3
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

/* parse_fragment_params */
static int parse_fragment_params(opttran_list_t *fragments_p, FILE *inputfile_p) {
    fragment_t *fragment;
    recommendation_t *recommendation;
    recognizer_t *recognizer;
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

            /* Create a list item for this code bottleneck */
            fragment = (fragment_t *)malloc(sizeof(fragment_t));
            if (NULL == fragment) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *)fragment);

            /* Initialize some elements on segment */
            fragment->filename = NULL;
            fragment->line_number = 0;
            fragment->fragment_file = NULL;
            opttran_list_construct((opttran_list_t *)&(fragment->recommendations));

            /* Add this item to 'segments' */
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

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            fragment->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->filename) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(fragment->filename, strlen(node->value) + 1);
            strcpy(fragment->filename, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  \\- %s            [%s]",
                                    input_line, _MAGENTA("filename:"),
                                    fragment->filename));
            free(node);
            continue;
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            fragment->line_number = atoi(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  \\- %s         [%d]", input_line,
                                    _MAGENTA("line number:"),
                                    fragment->line_number));
            free(node);
            continue;
        }
        /* Recommender param: recommender.code_fragment */
        if (0 == strncmp("recommender.code_fragment", node->key, 25)) {
            fragment->fragment_file = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->fragment_file) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(fragment->fragment_file, strlen(node->value) + 1);
            strcpy(fragment->fragment_file, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  \\- %s       [%s]", input_line,
                                    _MAGENTA("fragment file:"),
                                    fragment->fragment_file));
            free(node);
            continue;
        }
        /* It is expected that after a 'recommender.recommendation_id' there
         * will be a list of recognizer (no recognizers is also valid).
         */
        /* Recommender param: recommender.recommendation_id */
        if (0 == strncmp("recommender.recommendation_id", node->key, 29)) {
            recommendation = (recommendation_t *)malloc(sizeof(recommendation_t));
            if (NULL == recommendation) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *)recommendation);
            recommendation->id = atoi(node->value);
            opttran_list_construct((opttran_list_t *)&(recommendation->recognizers));
            opttran_list_append((opttran_list_t *)&(fragment->recommendations),
                                (opttran_list_item_t *)recommendation);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  | \\- %s [%d]", input_line,
                                    _YELLOW("recommendation ID:"),
                                    recommendation->id));
            free(node);
            continue;
        }
        /* If there is a new recognizer, the ID should become first, them the
         * recognizer name. This allow us to correctly allocate the struct mem.
         */
        /* Recommender param: recommender.recognizer_id */
        if (0 == strncmp("recommender.recognizer_id", node->key, 25)) {
            recognizer = (recognizer_t *)malloc(sizeof(recognizer_t));
            if (NULL == recognizer) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            opttran_list_item_construct((opttran_list_item_t *)recognizer);
            recognizer->id = atoi(node->value);
            recognizer->test_result = OPTTRAN_UNDEFINED;
            opttran_list_append((opttran_list_t *)&(recommendation->recognizers),
                                (opttran_list_item_t *)recognizer);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  | | \\- %s   [%d]", input_line,
                                    _CYAN("recognizer ID:"),
                                    recognizer->id));
            free(node);
            continue;
        }
        /* Recommender param: recommender.recognizer */
        if (0 == strncmp("recommender.recognizer", node->key, 22)) {
            recognizer->program = (char *)malloc(strlen(node->value) + 1);
            if (NULL == recognizer->program) {
                OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(OPTTRAN_ERROR);
            }
            bzero(recognizer->program, strlen(node->value) + 1);
            strcpy(recognizer->program, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d)  | | \\- %s      [%s]",
                                    input_line, _CYAN("recognizer:"),
                                    recognizer->program));
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

    /* print a summary of 'segments' */
    OPTTRAN_OUTPUT_VERBOSE((4, "%d %s", opttran_list_get_size(fragments_p),
                            _GREEN("code fragment(s) found")));

    fragment = (fragment_t *)opttran_list_get_first(fragments_p);
    while ((opttran_list_item_t *)fragment != &(fragments_p->sentinel)) {
        OPTTRAN_OUTPUT_VERBOSE((4, "   %s:%d", fragment->filename,
                                fragment->line_number));
        fragment = (fragment_t *)opttran_list_get_next(fragment);
    }

    OPTTRAN_OUTPUT_VERBOSE((4, "==="));

    return OPTTRAN_SUCCESS;
}

#if HAVE_SQLITE3
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
