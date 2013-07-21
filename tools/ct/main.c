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
#include <inttypes.h>

#if HAVE_SQLITE3 == 1
/* Utility headers */
#include <sqlite3.h>
#endif

/* PerfExpert headers */
#include "config.h"
#include "ct.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

// TODO: check for memory de-alocation on this entire code

/* main, life starts here */
int main(int argc, char** argv) {
    fragment_t *fragment;
    perfexpert_list_t *fragments;
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
        .workdir          = NULL,   // char *
        .transfall        = 0,      // int
#if HAVE_SQLITE3 == 1
        .perfexpert_pid   = (unsigned long long int)getpid(), // int
#endif
        .colorful         = 0       // int
    };
    globals.dbfile = (char *)malloc(strlen(RECOMMENDATION_DB) +
                                    strlen(PERFEXPERT_VARDIR) + 2);
    if (NULL == globals.dbfile) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(PERFEXPERT_ERROR);
    }
    bzero(globals.dbfile,
          strlen(RECOMMENDATION_DB) + strlen(PERFEXPERT_VARDIR) + 2);
    sprintf(globals.dbfile, "%s/%s", PERFEXPERT_VARDIR, RECOMMENDATION_DB);

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        exit(PERFEXPERT_ERROR);
    }

    /* Create the list of fragments */
    fragments = (perfexpert_list_t *)malloc(sizeof(perfexpert_list_t));
    if (NULL == fragments) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(PERFEXPERT_ERROR);
    }
    perfexpert_list_construct(fragments);

    /* Parse input parameters */
    if (1 == globals.use_stdin) {
        if (PERFEXPERT_SUCCESS != parse_transformation_params(fragments, stdin)) {
            OUTPUT(("%s", _ERROR("Error: parsing input params")));
            exit(PERFEXPERT_ERROR);
        }
    } else {
        if (NULL != globals.inputfile) {
            FILE *inputfile_FP = NULL;

            /* Open input file */
            if (NULL == (inputfile_FP = fopen(globals.inputfile, "r"))) {
                OUTPUT(("%s (%s)", _ERROR("Error: unable to open input file"),
                        globals.inputfile));
                return PERFEXPERT_ERROR;
            } else {
                if (PERFEXPERT_SUCCESS != parse_transformation_params(fragments,
                                                                      inputfile_FP)) {
                    OUTPUT(("%s", _ERROR("Error: parsing input params")));
                    exit(PERFEXPERT_ERROR);
                }
                fclose(inputfile_FP);
            }
        } else {
            OUTPUT(("%s", _ERROR("Error: undefined input")));
            show_help();
        }
    }

    /* Apply transformation */
    if (PERFEXPERT_SUCCESS != apply_transformations(fragments)) {
        OUTPUT(("%s", _ERROR("Error: applying transformations")));
        exit(PERFEXPERT_ERROR);
    }

    /* Output results */
    if (1 == globals.automatic) {
        globals.use_stdout = 0;

        if (NULL == globals.workdir) {
            globals.workdir = (char *)malloc(strlen("./perfexpert-") + 8);
            if (NULL == globals.workdir) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }
            bzero(globals.workdir, strlen("./perfexpert-" + 8));
            sprintf(globals.workdir, "./perfexpert-%d", getpid());
        }
        OUTPUT_VERBOSE((7, "using (%s) as output directory",
                        globals.workdir));

        if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.workdir, 0755)) {
            OUTPUT(("%s", _ERROR("Error: cannot create temporary directory")));
            exit(PERFEXPERT_ERROR);
        }

        globals.outputfile = (char *)malloc(strlen(globals.workdir) +
                                            strlen(PERFEXPERT_CT_FILE) + 2);
        if (NULL == globals.outputfile) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            exit(PERFEXPERT_ERROR);
        }
        bzero(globals.outputfile, strlen(globals.workdir) +
              strlen(PERFEXPERT_CT_FILE) + 2);
        strcat(globals.outputfile, globals.workdir);
        strcat(globals.outputfile, "/");
        strcat(globals.outputfile, PERFEXPERT_CT_FILE);
        OUTPUT_VERBOSE((7, "printing output to (%s)", globals.workdir));
    }

    if (0 == globals.use_stdout) {
        OUTPUT_VERBOSE((7, "printing test results to file (%s)",
                        globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OUTPUT(("%s (%s)", _ERROR("Error: unable to open output file"),
                    globals.outputfile));
            return PERFEXPERT_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((7, "printing test results to STDOUT"));
    }

    if (PERFEXPERT_SUCCESS != output_results(fragments)) {
        OUTPUT(("%s", _ERROR("Error: outputting results")));
        exit(PERFEXPERT_ERROR);
    }
    if (0 == globals.use_stdout) {
        fclose(globals.outputfile_FP);
    }

    /* Free memory */
    while (PERFEXPERT_FALSE == perfexpert_list_is_empty(fragments)) {
        fragment = (fragment_t *)perfexpert_list_get_first(fragments);
        perfexpert_list_remove_item(fragments, (perfexpert_list_item_t *)fragment);
        while (PERFEXPERT_FALSE == perfexpert_list_is_empty(&(fragment->transformations))) {
            transformation = (transformation_t *)perfexpert_list_get_first(&(fragment->transformations));
            perfexpert_list_remove_item(&(fragment->transformations),
                                        (perfexpert_list_item_t *)transformation);
            free(transformation->program);
            free(transformation->fragment_file);
            free(transformation);
        }
        free(fragment);
    }
    perfexpert_list_destruct(fragments);
    free(fragments);
    if (1 == globals.automatic) {
        free(globals.outputfile);
    }

    return PERFEXPERT_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OUTPUT_VERBOSE((10, "printing help"));

    /*      12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("Usage: perfexpert_ct -i|-f file [-o file] [-vch] [-l level] [-a dir]");
#if HAVE_SQLITE3 == 1
    printf(" [-d database] [-p pid]");
#endif
    printf("\n");
    printf("  -i --stdin           Use STDIN as input for patterns\n");
    printf("  -f --inputfile       Use 'file' as input for patterns\n");
    printf("  -o --outputfile      Use 'file' as output (default stdout)\n");
    printf("  -a --automatic       Use automatic performance optimization and create files\n");
    printf("                       into 'dir' directory (default: off).\n");
    printf("                       This argument overwrites -o (no output on STDOUT, except\n");
    printf("                       for verbose messages)\n");
    printf("  -t --transfall       Apply all possible transformation to each fragments (not\n");
    printf("                       recommended, some transformations are not compatible)\n");
#if HAVE_SQLITE3 == 1
    printf("  -d --database        Select the recommendation database file\n");
    printf("                       (default: %s/%s)\n", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
    printf("  -p --perfexpert_pid  Use 'pid' to log on DB consecutive calls to Recommender\n");
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
    exit(PERFEXPERT_ERROR);
}

/* parse_env_vars */
static int parse_env_vars(void) {
    // TODO: add here all the parameter we have for command line arguments
    char *temp_str;

    /* Get the variables */
    temp_str = getenv("PERFEXPERT_VERBOSE_LEVEL");
    if (NULL != temp_str) {
        globals.verbose_level = atoi(temp_str);
        if (0 != globals.verbose_level) {
            OUTPUT_VERBOSE((5, "ENV: verbose_level=%d", globals.verbose_level));
        }
    }

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Environment variables")));

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
        exit(PERFEXPERT_ERROR);
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
                OUTPUT_VERBOSE((10, "option 'l' set"));
                if (0 >= atoi(optarg)) {
                    OUTPUT(("%s (%d)",
                            _ERROR("Error: invalid debug level (too low)"),
                            atoi(optarg)));
                    show_help();
                }
                if (10 < atoi(optarg)) {
                    OUTPUT(("%s (%d)",
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
                OUTPUT_VERBOSE((10, "option 'v' set"));
                break;
#if HAVE_SQLITE3 == 1
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
#endif
            /* Activate colorful mode */
            case 'c':
                globals.colorful = 1;
                OUTPUT_VERBOSE((10, "option 'c' set"));
                break;

            /* Show help */
            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set"));
                show_help();

            /* Use STDIN? */
            case 'i':
                globals.use_stdin = 1;
                OUTPUT_VERBOSE((10, "option 'i' set"));
                break;

            /* Use input file? */
            case 'f':
                globals.use_stdin = 0;
                globals.inputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'f' set [%s]",
                                        globals.inputfile));
                break;

            /* Use output file? */
            case 'o':
                globals.use_stdout = 0;
                globals.outputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'o' set [%s]",
                                        globals.outputfile));
                break;

            /* Use automatic optimization? */
            case 'a':
                globals.automatic = 1;
                globals.use_stdout = 0;
                globals.workdir = optarg;
                OUTPUT_VERBOSE((10, "option 'a' set [%s]", globals.workdir));
                break;

            /* Apply all possible transformations for each code fragment? */
            case 't':
                globals.transfall = 1;
                OUTPUT_VERBOSE((10, "option 't' set"));
                break;


            /* Unknown option */
            case '?':
                show_help();

            default:
                exit(PERFEXPERT_ERROR);
        }
    }
    OUTPUT_VERBOSE((4, "=== %s", _BLUE("CLI params")));
    OUTPUT_VERBOSE((10, "Summary of selected options:"));
    OUTPUT_VERBOSE((10, "   Verbose:                    %s",
                    globals.verbose ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Verbose level:              %d",
                    globals.verbose_level));
    OUTPUT_VERBOSE((10, "   Colorful verbose?           %s",
                    globals.colorful ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Use STDOUT?                 %s",
                    globals.use_stdout ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Use STDIN?                  %s",
                    globals.use_stdin ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   Input file:                 %s",
                    globals.inputfile ? globals.inputfile : "(null)"));
    OUTPUT_VERBOSE((10, "   Output file:                %s",
                    globals.outputfile ? globals.outputfile : "(null)"));
    OUTPUT_VERBOSE((10, "   Use automatic optimization? %s",
                    globals.automatic ? "yes" : "no"));
    OUTPUT_VERBOSE((10, "   PerfExpert PID:             %llu",
                    globals.perfexpert_pid));
    OUTPUT_VERBOSE((10, "   Temporary directory:        %s",
                    globals.workdir ? globals.workdir : "(null)"));
    OUTPUT_VERBOSE((10, "   Apply all transf.?          %s",
                    globals.transfall ? "yes" : "no"));
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
static int parse_transformation_params(perfexpert_list_t *fragments_p,
                                       FILE *inputfile_p) {
    fragment_t *fragment;
    transformation_t *transformation;
    char buffer[BUFFER_SIZE];
    int  input_line = 0;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing fragments file")));

    /* Which INPUT we are using? (just a double check) */
    if ((NULL == inputfile_p) && (globals.use_stdin)) {
        inputfile_p = stdin;
    }
    if (globals.use_stdin) {
        OUTPUT_VERBOSE((3, "using STDIN as input for fragments"));
    } else {
        OUTPUT_VERBOSE((3, "using (%s) as input for fragments",
                        globals.inputfile));
    }

    /* For each line in the INPUT file... */
    OUTPUT_VERBOSE((7, "--- parsing input file"));

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

            OUTPUT_VERBOSE((5, "(%d) --- %s", input_line,
                            _GREEN("new bottleneck found")));

            /* Create a list item for this code fragment */
            fragment = (fragment_t *)malloc(sizeof(fragment_t));
            if (NULL == fragment) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *)fragment);

            /* Initialize some elements on 'fragment' */
            perfexpert_list_construct((perfexpert_list_t *)&(fragment->transformations));

            /* Add this item to 'fragments_p' */
            perfexpert_list_append(fragments_p, (perfexpert_list_item_t *)fragment);

            continue;
        }

        node = (node_t *)malloc(sizeof(node_t) + strlen(buffer) + 1);
        if (NULL == node) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            exit(PERFEXPERT_ERROR);
        }
        bzero(node, sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            fragment->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == fragment->filename) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
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
                exit(PERFEXPERT_ERROR);
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
                exit(PERFEXPERT_ERROR);
            }
            bzero(fragment->function_name, strlen(node->value) + 1);
            strcpy(fragment->function_name, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%s]", input_line,
                            _MAGENTA("function name:"),
                            fragment->function_name));
            free(node);
            continue;
        }

        /* OK, now it is time to check which parameter is this, and add it to
         * 'patterns'. I expect that for each 'recommender.code_fragment' will
         * have a correspondent 'pr.transformation'. The code fragment should
         * become first.
         */

        /* Code param: recommender.code_fragment */
        if (0 == strncmp("recommender.code_fragment", node->key, 25)) {
            transformation = (transformation_t *)malloc(sizeof(transformation_t));
            if (NULL == transformation) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *)transformation);
            perfexpert_list_append((perfexpert_list_t *)&(fragment->transformations),
                                   (perfexpert_list_item_t *)transformation);

            transformation->program = NULL;
            transformation->fragment_file = NULL;
            transformation->transf_function = NULL;
            transformation->line_number = 0;
            transformation->transf_result = PERFEXPERT_UNDEFINED;

            transformation->fragment_file = (char *)malloc(strlen(node->value) + 1);
            if (NULL == transformation->fragment_file) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }
            bzero(transformation->fragment_file, strlen(node->value) + 1);
            strcpy(transformation->fragment_file, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s  [%s]", input_line,
                            _MAGENTA("fragment file:"),
                            transformation->fragment_file));
            free(node);
            continue;
        }
        /* Code param: recommender.line_number */
        if (0 == strncmp("recommender.line_number", node->key, 23)) {
            transformation->line_number = atoi(node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%d]", input_line,
                            _MAGENTA("line number:"),
                            transformation->line_number));
            free(node);
            continue;
        }
        /* Code param: pr.transformation */
        if (0 == strncmp("pr.transformation", node->key, 17)) {
            transformation->program = (char *)malloc(strlen(node->value) + 1);
            if (NULL == transformation->program) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }

            bzero(transformation->program, strlen(node->value) + 1);
            strcpy(transformation->program, node->value);
            OUTPUT_VERBOSE((10, "(%d) %s [%s]", input_line,
                            _YELLOW("transformation:"),
                            transformation->program));
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
    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(fragments_p),
                    _GREEN("code bottleneck(s) found")));

    fragment = (fragment_t *)perfexpert_list_get_first(fragments_p);
    while ((perfexpert_list_item_t *)fragment != &(fragments_p->sentinel)) {
        transformation = (transformation_t *)perfexpert_list_get_first(&(fragment->transformations));
        while ((perfexpert_list_item_t *)transformation != &(fragment->transformations.sentinel)) {
            OUTPUT_VERBOSE((4, "   [%s] [%s]", transformation->program,
                            transformation->fragment_file));
            transformation = (transformation_t *)perfexpert_list_get_next(transformation);
        }
        fragment = (fragment_t *)perfexpert_list_get_next(fragment);
    }

    OUTPUT_VERBOSE((4, "==="));

    return PERFEXPERT_SUCCESS;
}

#if HAVE_SQLITE3 == 1
/* database_connect */
static int database_connect(void) {
    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Connecting to database")));

    /* Connect to the DB */
    if (NULL == globals.dbfile) {
        globals.dbfile = "./recommendation.db";
    }
    if (-1 == access(globals.dbfile, F_OK)) {
        OUTPUT(("%s (%s)",
                _ERROR("Error: recommendation database doesn't exist"),
                globals.dbfile));
        return PERFEXPERT_ERROR;
    }
    if (-1 == access(globals.dbfile, R_OK)) {
        OUTPUT(("%s (%s)", _ERROR("Error: you don't have permission to read"),
                globals.dbfile));
        return PERFEXPERT_ERROR;
    }

    if (SQLITE_OK != sqlite3_open(globals.dbfile, &(globals.db))) {
        OUTPUT(("%s (%s), %s", _ERROR("Error: openning database"),
                globals.dbfile, sqlite3_errmsg(globals.db)));
        sqlite3_close(globals.db);
        exit(PERFEXPERT_ERROR);
    } else {
        OUTPUT_VERBOSE((4, "connected to %s", globals.dbfile));
    }
    return PERFEXPERT_SUCCESS;
}
#endif

/* apply_transformations */
static int apply_transformations(perfexpert_list_t *fragments_p) {
    transformation_t *transformation;
    fragment_t *fragment;
//    perfexpert_list_t *transfs;
    perfexpert_list_t transfs;
    transf_t *transf;
    int fragment_id = 0;

//    transfs = (perfexpert_list_t *)malloc(sizeof(perfexpert_list_t));
//    if (NULL == transfs) {
//        OUTPUT(("%s", _ERROR("Error: out of memory")));
//        exit(PERFEXPERT_ERROR);
//    }
    perfexpert_list_construct(&transfs);

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Applying transformations")));

    OUTPUT_VERBOSE((8, "creating a list of transformations to apply..."));

    /* Create a list of all pattern recognizers we have to test */
    fragment = (fragment_t *)perfexpert_list_get_first(fragments_p);
    while ((perfexpert_list_item_t *)fragment != &(fragments_p->sentinel)) {
        /* For all code fragments ... */
        fragment_id++;
        transformation = (transformation_t *)perfexpert_list_get_first(&(fragment->transformations));
        while ((perfexpert_list_item_t *)transformation != &(fragment->transformations.sentinel)) {
            /* For all transformations ... */
            transf = (transf_t *)malloc(sizeof(transf_t));
            if (NULL == transf) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                exit(PERFEXPERT_ERROR);
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *)transf);

            transf->program              = transformation->program;
            transf->fragment_file        = transformation->fragment_file;
            transf->fragment_line_number = transformation->line_number;
            transf->filename             = fragment->filename;
            transf->line_number          = fragment->line_number;
            transf->code_type            = fragment->code_type;
            transf->function_name        = fragment->function_name;
            transf->transf_result        = &(transformation->transf_result);
            transf->transf_function      = &(transformation->transf_function);
            transf->fragment_id          = fragment_id;

            OUTPUT_VERBOSE((10, "[%s] %s", transf->program,
                            transf->fragment_file));

            /* Add this item to to-'tests' */
            perfexpert_list_append(&transfs, (perfexpert_list_item_t *)transf);

            transformation = (transformation_t *)perfexpert_list_get_next(transformation);
        }
        fragment = (fragment_t *)perfexpert_list_get_next(fragment);
    }
    OUTPUT_VERBOSE((8, "...done!"));

    /* Print a summary of 'tests' */
    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(&transfs),
                    _GREEN("possible transformation(s) found")));

    if (0 < perfexpert_list_get_size(&transfs)) {
   
        /* Apply the transformations */
        fragment_id = 0;
        transf = (transf_t *)perfexpert_list_get_first(&transfs);
        while ((perfexpert_list_item_t *)transf != &(transfs.sentinel)) {
            *(transf->transf_result) = PERFEXPERT_UNDEFINED;
            /* Skip this test if 'transfall' is not set */
            if ((0 == globals.transfall) && (fragment_id >= transf->fragment_id)) {
                OUTPUT(("   %s  [%s] >> [%s]", _MAGENTA("SKIP"), transf->program,
                        transf->filename));
                transf = (transf_t *)perfexpert_list_get_next(transf);
                continue;
            }

            if (PERFEXPERT_SUCCESS != apply_one(transf)) {
                OUTPUT(("   %s [%s] >> [%s]",
                        _RED("Error: running code transformer"), transf->program,
                        transf->filename));
            }

            switch ((int)*(transf->transf_result)) {
                case PERFEXPERT_UNDEFINED:
                    OUTPUT_VERBOSE((8, "   %s [%s] >> [%s]", _BOLDRED("UNDEF"),
                                    transf->program, transf->filename));
                    break;

                case PERFEXPERT_FAILURE:
                    OUTPUT_VERBOSE((8, "   %s  [%s] >> [%s]", _ERROR("FAIL"),
                                    transf->program, transf->filename));
                    break;

                case PERFEXPERT_SUCCESS:
                    OUTPUT_VERBOSE((8, "   %s    [%s] >> [%s]", _BOLDGREEN("OK"),
                                    transf->program, transf->filename));
                    fragment_id = transf->fragment_id;
                    break;

                case PERFEXPERT_ERROR:
                    OUTPUT_VERBOSE((8, "   %s [%s] >> [%s]", _BOLDYELLOW("ERROR"),
                                transf->program, transf->filename));
                    break;

                default:
                    break;
            }

            /* Move on to the next test... */
            transf = (transf_t *)perfexpert_list_get_next(transf);
        }
        /* Free 'transfs' structure' */
        while (PERFEXPERT_FALSE == perfexpert_list_is_empty(&transfs)) {
            transf = (transf_t *)perfexpert_list_get_first(&transfs);
            perfexpert_list_remove_item(&transfs, (perfexpert_list_item_t *)transf);
            free(transf);
        }
    }

    perfexpert_list_destruct(&transfs);
//    free(transfs);
    OUTPUT_VERBOSE((4, "==="));
    return PERFEXPERT_SUCCESS;
}

/* apply_one */
static int apply_one(transf_t *transf) {
    int  pid = 0;
    int  rc = PERFEXPERT_UNDEFINED;
    char temp_str[BUFFER_SIZE];
    char temp_str2[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    char argv[20][PARAM_SIZE];

    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/ct_%s", PERFEXPERT_BINDIR, transf->program);

    /* Set the code transformer arguments. Ok, we have to define an
     * interface to code transformers. Here is a simple one. Each code
     * transformer will be called using the following arguments:
     *
     * -c TYPE      Code type, basically there are two options: "loop" and
     *              "function"
     * -d           Enable debug mode and write LOG to FILE defined with -o
     * -f FUNCTION  Function name were code bottleneck belongs to
     * -l LINE      Line number identified by HPCtoolkit/PerfExpert/etc...
     * -o FILE      Output file, considering that output is only verbose
     *              messages, not code
     * -p NAME      Project name, which is the name of the recognizer
     * -r FILE      File (maybe link) containing the transformation result
     * -s FILE      Source file
     * -w DIR       Use DIR as work directory
     */
    bzero(argv, PARAM_SIZE * 20);
    sprintf(argv[0], "ct_%s", transf->program);
    sprintf(argv[1], "-c");
    sprintf(argv[2], "%s", transf->code_type);
    sprintf(argv[3], "-d");
    sprintf(argv[4], "-f");
    sprintf(argv[5], "%s", transf->function_name);
    sprintf(argv[6], "-l");
    sprintf(argv[7], "%d", transf->fragment_line_number);
    sprintf(argv[8], "-o");
    sprintf(argv[9], "%s_%d.%s.transformer_output", transf->filename,
            transf->line_number, transf->program);
    sprintf(argv[10], "-p");
    sprintf(argv[11], "%s", transf->program);
    sprintf(argv[12], "-r");
    // sprintf(argv[13], "%s_%d.%s.transformer_result", transf->filename,
    //         transf->line_number, transf->program);
    sprintf(argv[13], "new_%s", transf->filename);
    sprintf(argv[14], "-s");
    // sprintf(argv[15], "../%s/%s", PERFEXPERT_SOURCE_DIR, transf->filename);
    sprintf(argv[15], "%s", transf->filename);
    sprintf(argv[16], "-w");
    // sprintf(argv[17], "%s/%s", globals.workdir, PERFEXPERT_FRAGMENTS_DIR);
    sprintf(argv[17], "./");

    /* Setting the output */
    *(transf->transf_function) = (char *)malloc(strlen(argv[17]) +
                                                strlen(argv[13]) + 2);
    if (NULL == transf->transf_function) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(PERFEXPERT_ERROR);
    }
    bzero(*(transf->transf_function), (strlen(argv[17]) +
                                       strlen(argv[13]) + 2));
    sprintf(*(transf->transf_function), "%s/%s", argv[17], argv[13]);

    OUTPUT_VERBOSE((10, "   output  %s", _CYAN(*(transf->transf_function))));

    /* Set the command line */
    bzero(temp_str2, BUFFER_SIZE);
    sprintf(temp_str2,
            "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
            temp_str, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5],
            argv[6], argv[7], argv[8], argv[9], argv[10], argv[11],
            argv[12], argv[13], argv[14], argv[15], argv[16], argv[17]);
    OUTPUT_VERBOSE((10, "   running %s", _CYAN(temp_str2)));

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
              argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12],
              argv[13], argv[14], argv[15], argv[16], argv[17], NULL);

        OUTPUT(("child process failed to run, check if program exists"));
        exit(127);
    } else {
        /* Parent */
        wait(&rc);
        OUTPUT_VERBOSE((10, "   result  %s %d", _CYAN("return code"), rc >> 8));
    }

    /* Evaluating the result */
    switch (rc >> 8) {
        /* The transformation was possible */
        case 0:
            *(transf->transf_result) = PERFEXPERT_SUCCESS;
            break;

        /* The transformation was not possible */
        case 255:
            *(transf->transf_result) = PERFEXPERT_ERROR;
            break;

        /* Error during fork() or waitpid() */
        case -1:
            *(transf->transf_result) = PERFEXPERT_FAILURE;
            break;

        /* Execution failed */
        case 127:
            *(transf->transf_result) = PERFEXPERT_FAILURE;
            break;

        /* Not sure what happened */
        default:
            *(transf->transf_result) = PERFEXPERT_UNDEFINED;
            break;
    }

    return PERFEXPERT_SUCCESS;
}

// TODO: insert results on DB
/* output results */
static int output_results(perfexpert_list_t *fragments_p) {
    transformation_t *transformation;
    fragment_t *fragment;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Outputting results")));

    /* Output transformation results */
    fragment = (fragment_t *)perfexpert_list_get_first(fragments_p);
    while ((perfexpert_list_item_t *)fragment != &(fragments_p->sentinel)) {
        /* For all code fragments ... */
        transformation = (transformation_t *)perfexpert_list_get_first(&(fragment->transformations));
        while ((perfexpert_list_item_t *)transformation != &(fragment->transformations.sentinel)) {
            /* For all transformations ... */
            if (PERFEXPERT_SUCCESS == transformation->transf_result) {
                if (0 == globals.use_stdout) {
                    fprintf(globals.outputfile_FP,
                            "%% function replacement for %s:%d\n",
                            fragment->filename, fragment->line_number);
                    fprintf(globals.outputfile_FP, "code.filename=%s/%s/%s\n",
                            globals.workdir, PERFEXPERT_SOURCE_DIR,
                            fragment->filename);
                    fprintf(globals.outputfile_FP, "code.function_name=%s\n",
                            fragment->function_name);
                    fprintf(globals.outputfile_FP,
                            "perfexpert_ct.replacement_function=%s\n",
                            transformation->transf_function);
                } else {
                    fprintf(globals.outputfile_FP,
                            "#--------------------------------------------------\n");
                    fprintf(globals.outputfile_FP,
                            "# Function replacement for %s:%d\n",
                            fragment->filename, fragment->line_number);
                    fprintf(globals.outputfile_FP,
                            "#--------------------------------------------------\n");
                    fprintf(globals.outputfile_FP, "Filename : %s\n",
                            fragment->filename);
                    fprintf(globals.outputfile_FP, "Function Name: %s\n",
                            fragment->function_name);
                    fprintf(globals.outputfile_FP,
                            "Replacement Function Filename : %s\n",
                            transformation->transf_function);
                }
            }
            transformation = (transformation_t *)perfexpert_list_get_next(transformation);
        }
        fragment = (fragment_t *)perfexpert_list_get_next(fragment);
    }

    OUTPUT_VERBOSE((4, "==="));
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
