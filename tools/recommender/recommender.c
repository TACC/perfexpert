/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include <sqlite3.h>
#include "config.h"
#include "recommender.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

int main (int argc, char** argv) {
    opttran_list_t segments;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose       = 0,                     // int
        .verbose_level = 0,                     // int
        .use_stdin     = 0,                     // int
        .use_stdout    = 1,                     // int
        .use_opttran   = 0,                     // int
        .inputfile     = NULL,                  // char *
        .outputfile    = NULL,                  // char *
        .dbfile        = "./recommendation.db", // char *
        .opttrandir    = NULL                   // char *
    };

    /* Parse command-line parameters */
    parse_cli_params(argc, argv);
    
    /* Create the list of code bottlenecks */
    opttran_list_construct(&(segments));
    
    /* Parse input parameters */
    if (globals.use_stdin) {
        OPTTRAN_OUTPUT_VERBOSE((3, "[recommender] using STDIN as default input for performance measurements"));
        parse_segment_params(&segments, NULL);
    } else {
        if (NULL != globals.inputfile) {
            FILE *inputfile = NULL;
            
            OPTTRAN_OUTPUT_VERBOSE((3, "[recommender] using FILE (%s) as default input for performance measurements",
                                    globals.inputfile));
            
            /* Open input file */
            if (NULL == (inputfile = fopen(globals.inputfile, "r"))) {
                OPTTRAN_OUTPUT(("[recommender] error openning input file (%s)",
                                globals.inputfile));
                return OPTTRAN_ERROR;
            } else {
                parse_segment_params(&segments, inputfile);
            }
        } else {
            fprintf(stderr, "Error: undefined input\n");
            show_help();
        }
    }
    
    /* Calculate the weigths for each code bottleneck */
    calculate_weigths();
    
    /* Select/output recommendations for each code bottleneck */
    select_recommendations();
    
    /* Free segments (and all items) structure */
    while (OPTTRAN_FALSE == opttran_list_is_empty(&(segments))) {
        segment_t *item;
        item = (segment_t *)opttran_list_get_first(&(segments));
        opttran_list_remove_item(&(segments), (opttran_list_item_t *)item);
        
        free(item->filename);
        free(item->type);
        free(item->extra_info);
        free(item->section_info);
        free(item);
    } // I hope I didn't left anything behind...
    
    return OPTTRAN_SUCCESS;
}

/* show_help */
static void show_help(void) {
    OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] printing help"));
    
    /*               12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    OPTTRAN_OUTPUT(("Usage: recommender -i|-f file [-o file] [-d database] [-a dir] [-hv] [-l level]"));
    OPTTRAN_OUTPUT(("  -i --stdin         Use STDIN as input for performance measurements"));
    OPTTRAN_OUTPUT(("  -f --inputfile     Use 'file' as input for performance measurements"));
    OPTTRAN_OUTPUT(("  -o --outputfile    Use 'file' as output for recommendations (default: stdout)"));
    OPTTRAN_OUTPUT(("  -d --database      Select database file (default: ./recommendation.db)"));
    OPTTRAN_OUTPUT(("  -a --opttran       Create OptTran (automatic performance optimization) files"));
    OPTTRAN_OUTPUT(("                     into 'dir' directory (default: create no OptTran files)"));
    OPTTRAN_OUTPUT(("  -h --help          Show this message"));
    OPTTRAN_OUTPUT(("  -v --verbose       Enable verbose mode using default verbose level (5)"));
    OPTTRAN_OUTPUT(("  -l --verbose_level Enable verbose mode using a specific verbose level (1-10)"));
    
    /* I suppose that if I've to show the help is because something is wrong,
     * or maybe the user just want to see the options, so it seems to be a
     * good idea to exit the daemon with an error code.
     */
    exit(OPTTRAN_ERROR);
}

/* parse_env_vars */
static int parse_env_vars(void) {
    char *temp_str;
    
    /* Get the variables */
    temp_str = getenv("OPTTRAN_VERBOSE_LEVEL");
    if (NULL != temp_str) {
        globals.verbose_level = atoi(temp_str);
        if (0 != globals.verbose_level) {
            OPTTRAN_OUTPUT_VERBOSE((5, "[recommender] ENV: verbose_level=%d",
                                       globals.verbose_level));
        }
    }

    OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] === Environment variables OK ==="));

    return OPTTRAN_SUCCESS;
}

/* parse_cli_params */
static int parse_cli_params(int argc, char *argv[]) {
    /** Temporary variable to hold parameter */
    int parameter;
    /** getopt_long() stores the option index here */
    int option_index = 0;

    /* If some environment variable is defined, use it! */
    parse_env_vars();

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "vhil:f:d:o:a:", long_options,
                                &option_index);
        
        /* Detect the end of the options */
        if (-1 == parameter)
            break;
        
        switch (parameter) {
            /* Verbose level */
            case 'l':
                globals.verbose = 1;
                globals.verbose_level = atoi(optarg);
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'l' set"));
                if (0 >= atoi(optarg)) {
                    OPTTRAN_OUTPUT(("[recommender] invalid debug level: too low (%d)",
                                    atoi(optarg)));
                    fprintf(stderr, "Error: invalid debug level: too low (%d)\n",
                            atoi(optarg));
                    show_help();
                }
                if (10 < atoi(optarg)) {
                    OPTTRAN_OUTPUT(("[recommender] invalid debug level: too high (%d)",
                                    atoi(optarg)));
                    fprintf(stderr, "Error: invalid debug level: too high (%d)\n",
                            atoi(optarg));
                    show_help();
                }
                break;

            /* Show help */
            case 'h':
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'h' set"));
                show_help();

            /* Use STDIN? */
            case 'i':
                globals.use_stdin = 1;
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'i' set"));
                break;

            /* Use input file? */
            case 'f':
                globals.use_stdin = 0;
                globals.inputfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'f' set [%s]",
                                        globals.inputfile));
                break;

            /* Use output file? */
            case 'o':
                globals.use_stdout = 0;
                globals.outputfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'o' set [%s]",
                                        globals.outputfile));
                break;

            /* Use opttran? */
            case 'a':
                globals.use_opttran = 1;
                globals.opttrandir = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'a' set [%s]",
                                        globals.opttrandir));
                break;

            /* Which database file? */
            case 'd':
                globals.dbfile = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'd' set [%s]",
                                        globals.dbfile));
                break;
                
            /* Activate verbose mode */
            case 'v':
                globals.verbose = 1;
                if (0 == globals.verbose_level) {
                    globals.verbose_level = 5;
                }
                OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] option 'v' set"));
                break;
                
            /* Unknown option */
            case '?':
                show_help();
                
            default:
                exit(OPTTRAN_ERROR);
        }
    }
    OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] === CLI params OK ==="));
    return OPTTRAN_SUCCESS;
}

/* parse_segment_params */
static int parse_segment_params(opttran_list_t *segments_p, FILE *inputfile_p) {
    segment_t *item;
    int input_line = 0;
    char buffer[1024];
    
    OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] === Parsing measurements ==="));
    
    if ((NULL == inputfile_p) && (globals.use_stdin)) {
        inputfile_p = stdin;
    }
    
    while (NULL != fgets(buffer, sizeof buffer, inputfile_p)) {
        node_t *node;
        
        input_line++;

        /* Is this line a new code bottleneck specification? */
        if (0 == strncmp("%", buffer, 1)) {
            OPTTRAN_OUTPUT_VERBOSE((5, "[recommender] (%d) --- found new bottleneck",
                                    input_line));

            /* Create a list item for this code bottleneck */
            item = malloc(sizeof(segment_t));
            opttran_list_item_construct((opttran_list_item_t *) item);
            
            /* Add this item to 'segments' */
            opttran_list_append(segments_p, (opttran_list_item_t *) item);
            
            continue;
        }

        node = malloc(sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments'
         */
        
        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            item->filename = malloc(strlen(node->value) + 1);
            strcpy(item->filename, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%s], filename",
                                    input_line, item->filename));
            continue;
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            item->line_number = atoi(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%d], line number",
                                    input_line, item->line_number));
            continue;
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            item->type = malloc(strlen(node->value) + 1);
            strcpy(item->type, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%s], type",
                                    input_line, item->type));
            continue;
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            item->extra_info = malloc(strlen(node->value) + 1);
            strcpy(item->extra_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%s], extra info",
                                    input_line, item->extra_info));
            continue;
        }
        /* Code param: code.representativeness */
        if (0 == strncmp("code.representativeness", node->key, 23)) {
            item->representativeness = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], representativeness",
                                    input_line, item->representativeness));
            continue;
        }
        /* Code param: code.section_info */
        if (0 == strncmp("code.section_info", node->key, 17)) {
            item->section_info = malloc(strlen(node->value) + 1);
            strcpy(item->section_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%s], section info",
                                    input_line, item->section_info));
            continue;
        }

        /* PerfExpert Measurement: overall */
        if (0 == strncmp("overall", node->key, 7)) {
            item->data.overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], overall",
                                    input_line, item->data.overall));
            continue;
        }
        /* PerfExpert Measurement: data_accesses_overall */
        if (0 == strncmp("data_accesses.overall", node->key, 22)) {
            item->data.data_accesses_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data accesses overall",
                                    input_line, item->data.data_accesses_overall));
            continue;
        }
        /* PerfExpert Measurement: data_accesses_L1d_hits */
        if (0 == strncmp("data_accesses.L1d_hits", node->key, 22)) {
            item->data.data_accesses_L1d_hits = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data accesses L1d hits",
                                    input_line, item->data.data_accesses_L1d_hits));
            continue;
        }
        /* PerfExpert Measurement: data_accesses_L2d_hits */
        if (0 == strncmp("data_accesses.L2d_hits", node->key, 22)) {
            item->data.data_accesses_L2d_hits = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data accesses L2d hits",
                                    input_line, item->data.data_accesses_L2d_hits));
            continue;
        }
        /* PerfExpert Measurement: data_accesses_L2d_misses */
        if (0 == strncmp("data_accesses.L2d_misses", node->key, 24)) {
            item->data.data_accesses_L2d_misses = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data accesses L2d misses",
                                    input_line, item->data.data_accesses_L2d_misses));
            continue;
        }
        /* PerfExpert Measurement: data_accesses_L3d_misses */
        if (0 == strncmp("data_accesses.L3d_misses", node->key, 24)) {
            item->data.data_accesses_L3d_misses = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data accesses L3d misses",
                                    input_line, item->data.data_accesses_L3d_misses));
            continue;
        }
        /* PerfExpert Measurement: ratio_floating_point */
        if (0 == strncmp("ratio.floating_point", node->key, 20)) {
            item->data.ratio_floating_point = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], ratio floating point",
                                    input_line, item->data.ratio_floating_point));
            continue;
        }
        /* PerfExpert Measurement: ratio_data_accesses */
        if (0 == strncmp("ratio.data_accesses", node->key, 19)) {
            item->data.ratio_data_accesses = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], ratio data accesses",
                                    input_line, item->data.ratio_data_accesses));
            continue;
        }
        /* PerfExpert Measurement: instruction_accesses_overall */
        if (0 == strncmp("instruction_accesses.overall", node->key, 28)) {
            item->data.instruction_accesses_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], instruction accesses overall",
                                    input_line, item->data.instruction_accesses_overall));
            continue;
        }
        /* PerfExpert Measurement: instruction_accesses_L1i_hits */
        if (0 == strncmp("instruction_accesses.L1i_hits", node->key, 29)) {
            item->data.instruction_accesses_L1i_hits = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], instruction accesses L1i hits",
                                    input_line, item->data.instruction_accesses_L1i_hits));
            continue;
        }
        /* PerfExpert Measurement: instruction_accesses_L2i_hits */
        if (0 == strncmp("instruction_accesses.L2i_hits", node->key, 29)) {
            item->data.instruction_accesses_L2i_hits = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], instruction accesses L2i hits",
                                    input_line, item->data.instruction_accesses_L2i_hits));
            continue;
        }
        /* PerfExpert Measurement: instruction_accesses_L2i_misses */
        if (0 == strncmp("instruction_accesses.L2i_misses", node->key, 31)) {
            item->data.instruction_accesses_L2i_misses = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], instruction accesses L2i misses",
                                    input_line, item->data.instruction_accesses_L2i_misses));
            continue;
        }
        /* PerfExpert Measurement: instruction_TLB_overall */
        if (0 == strncmp("instruction_TLB.overall", node->key, 23)) {
            item->data.instruction_TLB_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], instruction TLB overall",
                                    input_line, item->data.instruction_TLB_overall));
            continue;
        }
        /* PerfExpert Measurement: data_TLB_overall */
        if (0 == strncmp("data_TLB.overall", node->key, 16)) {
            item->data.data_TLB_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], data TLB overall",
                                    input_line, item->data.data_TLB_overall));
            continue;
        }
        /* PerfExpert Measurement: branch_instructions_overall */
        if (0 == strncmp("branch_instructions.overall", node->key, 27)) {
            item->data.branch_instructions_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], branch instructions overall",
                                    input_line, item->data.branch_instructions_overall));
            continue;
        }
        /* PerfExpert Measurement: branch_instructions_correctly_predicted */
        if (0 == strncmp("branch_instructions.correctly_predicted", node->key, 39)) {
            item->data.branch_instructions_correctly_predicted = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], branch instructions correctly predicted",
                                    input_line, item->data.branch_instructions_correctly_predicted));
            continue;
        }
        /* PerfExpert Measurement: branch_instructions_mispredicted */
        if (0 == strncmp("branch_instructions.mispredicted", node->key, 32)) {
            item->data.branch_instructions_mispredicted = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], branch instructions mispredicted",
                                    input_line, item->data.branch_instructions_mispredicted));
            continue;
        }
        /* PerfExpert Measurement: floating_point_instr_overall */
        if (0 == strncmp("floating-point_instr.overall", node->key, 28)) {
            item->data.floating_point_instr_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], floating point instr overall",
                                    input_line, item->data.floating_point_instr_overall));
            continue;
        }
        /* PerfExpert Measurement: floating_point_instr_fast_FP_instr */
        if (0 == strncmp("floating-point_instr.fast_FP_instr", node->key, 34)) {
            item->data.floating_point_instr_fast_FP_instr = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], floating point instr fast FP instr",
                                    input_line, item->data.floating_point_instr_fast_FP_instr));
            continue;
        }
        /* PerfExpert Measurement: floating_point_instr_slow_FP_instr */
        if (0 == strncmp("floating-point_instr.slow_FP_instr", node->key, 34)) {
            item->data.floating_point_instr_slow_FP_instr = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], floating point instr slow FP instr",
                                    input_line, item->data.floating_point_instr_slow_FP_instr));
            continue;
        }
        /* PerfExpert Measurement: percent_GFLOPS_max_overall */
        if (0 == strncmp("percent.GFLOPS_(%_max).overall", node->key, 30)) {
            item->data.percent_GFLOPS_max_overall = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], percent GFLOPS (% max) overall",
                                    input_line, item->data.percent_GFLOPS_max_overall));
            continue;
        }
        /* PerfExpert Measurement: percent_GFLOPS_max_packed */
        if (0 == strncmp("percent.GFLOPS_(%_max).packed", node->key, 29)) {
            item->data.percent_GFLOPS_max_packed = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], percent GFLOPS (% max) packed",
                                    input_line, item->data.percent_GFLOPS_max_packed));
            continue;
        }
        /* PerfExpert Measurement: percent_GFLOPS_max_scalar */
        if (0 == strncmp("percent.GFLOPS_(%_max).scalar", node->key, 29)) {
            item->data.percent_GFLOPS_max_scalar = atof(node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "[recommender] (%d)   [%f], percent GFLOPS (% max) scalar",
                                    input_line, item->data.percent_GFLOPS_max_scalar));
            continue;
        }

        /* Unknown parameter */
        OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] (%d) ignored line [%s=%s]",
                                    input_line, node->key, node->value));
        
        free(node);
    }
    /* print a summary of 'segments' */
    OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] %d code segments found",
                            opttran_list_get_size(segments_p)));
    
    item = (segment_t *)opttran_list_get_first(segments_p);
    do {
        OPTTRAN_OUTPUT_VERBOSE((4, "[recommender]   %s:%d",
                                item->filename, item->line_number));
        item = (segment_t *)opttran_list_get_next(item);
    } while ((opttran_list_item_t *)item != &(segments_p->sentinel));
    
    OPTTRAN_OUTPUT_VERBOSE((4, "[recommender] === STDIN params OK ==="));
    return OPTTRAN_SUCCESS;
}

/* output_recommendations */
static int output_recommendations(void *NotUsed, int argc, char **argv,
                                  char **azColName){
    // TODO: make this output the correct information, this is just a test
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/* query_database */
static int query_database(void) {
    sqlite3 *database;
    
    if (NULL == globals.dbfile) {
        globals.dbfile = "./recommendation.db";
    }
    if (-1 == access(globals.dbfile, F_OK)) {
        OPTTRAN_OUTPUT(("[recommender] recommendation database (%s) doesn't exist",
                        globals.dbfile));
        return OPTTRAN_ERROR;
    }
    if (-1 == access(globals.dbfile, R_OK)) {
        OPTTRAN_OUTPUT(("[recommender] you don't have permission to read the recommendation database (%s)",
                        globals.dbfile));
        return OPTTRAN_ERROR;
    }
    if (SQLITE_OK != sqlite3_open(globals.dbfile, &database)) {
        OPTTRAN_OUTPUT(("[recommender] error openning recommendation database (%s), %s",
                        globals.dbfile, sqlite3_errmsg(database)));
        sqlite3_close(database);
        return OPTTRAN_ERROR;
    }
    
    // TODO: define que correct SQL query
    char *zErrMsg = 0;
    if (SQLITE_OK != sqlite3_exec(database, "SELECT * FROM attribute;",
                                  output_recommendations, 0, &zErrMsg)) {
        fprintf(stderr, "Error: SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(database);
    
    return OPTTRAN_SUCCESS;
}

/* calculate_weigths */
static int calculate_weigths(void) {
    // TODO: everything
    return OPTTRAN_SUCCESS;
}

/* select_recommendations */
static int select_recommendations(void) {
    query_database();
    // TODO: everything
    return OPTTRAN_SUCCESS;
}

// EOF