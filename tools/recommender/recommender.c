/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
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

/* Functions declaration */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_metrics_file(void);
static int  parse_segment_params(opttran_list_t *segments_p, FILE *inputfile_p);
static int  output_recommendations(void *not_used, int col_count,
                                   char **col_values, char **col_names);
static int  get_rowid(void *rowid, int col_count,
                      char **col_values, char **col_names);
static int  database_connect(void);
static int  database_query(void);
static int  calculate_weigths(void);
static int  select_recommendations(void);

/* main, life starts here */
int main (int argc, char** argv) {
    opttran_list_t segments;
    segment_t *item;
    int i;
    
    /* Set default values for globals */
    globals = (globals_t) {
        .verbose          = 0,                 // int
        .verbose_level    = 0,                 // int
        .use_stdin        = 0,                 // int
        .use_stdout       = 1,                 // int
        .use_opttran      = 0,                 // int
        .inputfile        = NULL,              // char *
        .outputfile       = NULL,              // char *
        .outputfile_FP    = stdout,            // FILE *
        .dbfile           = RECOMMENDATION_DB, // char *
        .opttrandir       = NULL,              // char *
        .metrics_file     = NULL,              // char *
        .use_temp_metrics = 0,                 // int
        .colorful         = 0,                 // int
        .metrics_table    = "metrics"          // char *
    };

    /* Parse command-line parameters */
    parse_cli_params(argc, argv);
    if (8 <= globals.verbose_level) {
        printf("%s complete command line:", PROGRAM_PREFIX);
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }
    
    /* Connect to database */
    database_connect();
    
    /* Create the list of code bottlenecks */
    opttran_list_construct(&(segments));
    
    /* Calculate the temporary metrics table name */
    if (1 == globals.use_temp_metrics) {
        globals.metrics_table = malloc(strlen("metrics_") + 6);
        sprintf(globals.metrics_table, "metrics_%d", (int)getpid());
    }
    
    /* Parse metrics file if 'm' is defined, this will create a temporary table */
    if (globals.metrics_file || (1 == globals.use_temp_metrics)) {
        parse_metrics_file();
    }
    
    /* Parse input parameters */
    if (globals.use_stdin) {
        parse_segment_params(&segments, stdin);
    } else {
        if (NULL != globals.inputfile) {
            FILE *inputfile_FP = NULL;
            
            /* Open input file */
            if (NULL == (inputfile_FP = fopen(globals.inputfile, "r"))) {
                OPTTRAN_OUTPUT(("%s (%s)", _ERROR("error openning input file"),
                                globals.inputfile));
                return OPTTRAN_ERROR;
            } else {
                parse_segment_params(&segments, inputfile_FP);
                fclose(inputfile_FP);
            }
        } else {
            OPTTRAN_OUTPUT(("%s", _ERROR("Error: undefined input")));
            show_help();
        }
    }
    
    /* Calculate the weigths for each code bottleneck */
    calculate_weigths();
    
    /* Select/output recommendations for each code bottleneck (4 steps) */
    /* Step 1: Print to a file or STDOUT is ok? Was OPRTRAN chosen? */
    if (1 == globals.use_opttran) {
        globals.use_stdout = 0;
        globals.outputfile = malloc(strlen(globals.opttrandir) +
                                    strlen(OPTTRAN_RECO_FILE) + 1);
        bzero(globals.outputfile, strlen(globals.opttrandir) +
              strlen(OPTTRAN_RECO_FILE) + 1);
        strcat(globals.outputfile, globals.opttrandir);
        strcat(globals.outputfile, "/");
        strcat(globals.outputfile, OPTTRAN_RECO_FILE);
        OPTTRAN_OUTPUT_VERBOSE((7, "printing OPTTRAN recommendation to dir (%s)",
                                globals.opttrandir));
    }
    if (0 == globals.use_stdout) {
        OPTTRAN_OUTPUT_VERBOSE((7, "printing recommendation to file (%s)",
                                globals.outputfile));
        globals.outputfile_FP = fopen(globals.outputfile, "w+");
        if (NULL == globals.outputfile_FP) {
            OPTTRAN_OUTPUT(("%s (%s)", _ERROR("error opening file"),
                            globals.outputfile));
            return OPTTRAN_ERROR;
        }
    } else {
        OPTTRAN_OUTPUT_VERBOSE((7, "printing recommendation to STDOUT"));
    }

    /* Step 2: For each code bottleneck... */
    item = (segment_t *)opttran_list_get_first(&(segments));
    do {
        OPTTRAN_OUTPUT_VERBOSE((4, "%s %s:%d",
                                _YELLOW("selecting recommendation for"),
                                item->filename, item->line_number));
        if (1 == globals.use_opttran) {
            fprintf(globals.outputfile_FP, "%% recommendation for %s:%d\n",
                    item->filename, item->line_number);
            fprintf(globals.outputfile_FP, "code.filename=%s\n",
                    item->filename);
            fprintf(globals.outputfile_FP, "code.line_number=%d\n",
                    item->line_number);
            fprintf(globals.outputfile_FP, "code.fragment_file=%s_%d\n",
                    item->filename, item->line_number);
        } else {
            fprintf(globals.outputfile_FP,
                    "#--------------------------------------------------\n");
            fprintf(globals.outputfile_FP,
                    "# Recommendations for %s:%d\n", item->filename,
                    item->line_number);
            fprintf(globals.outputfile_FP,
                    "#--------------------------------------------------\n");
        }

        /* Step 3: query DB for recommendations */
        select_recommendations();

        if (0 == globals.use_opttran) {
            fprintf(globals.outputfile_FP, "\n");
        }
            
        /* Move to the next code bottleneck */
        item = (segment_t *)opttran_list_get_next(item);
    } while ((opttran_list_item_t *)item != &(segments.sentinel));

    /* Step 4: If we are using output file, close it! (metrics DB too) */
    if (0 == globals.use_stdout) {
        fclose(globals.outputfile_FP);
    }
    sqlite3_close(globals.db);

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
    }
    if (1 == globals.use_opttran) {
        free(globals.outputfile);
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
    printf("Usage: recommender -i|-f file [-o file] [-d database] [-a dir] [-m file] [-hnvc]\n");
    printf("                   [-l level]\n");
    printf("  -i --stdin         Use STDIN as input for performance measurements\n");
    printf("  -f --inputfile     Use 'file' as input for performance measurements\n");
    printf("  -o --outputfile    Use 'file' as output for recommendations (default: stdout)\n");
    printf("                     if the file exists its content will be overwritten\n");
    printf("  -a --opttran       Create OptTran (automatic performance optimization) files\n");
    printf("                     into 'dir' directory (default: create no OptTran files)\n");
    printf("                     this argument overwrites -o, no output will be produced\n");
    printf("  -d --database      Select database file\n");
    printf("                     (default: %s)\n", RECOMMENDATION_DB);
    printf("  -m --metricfile    Use 'file' to define metrics different from the default\n");
    printf("  -n --newmetrics    Do not use the system metrics table. A temporary table will\n");
    printf("                     be created using the default metrics file:\n");
    printf("                     %s)\n", METRICS_FILE);
    printf("  -v --verbose       Enable verbose mode using default verbose level (5)\n");
    printf("  -l --verbose_level Enable verbose mode using a specific verbose level (1-10)\n");
    printf("  -c --colorful      Enable colors on verbose mode, no weird characters will\n");
    printf("                     apper on output files\n");
    printf("  -h --help          Show this message\n");
    
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
    parse_env_vars();

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "cvhinm:l:f:d:o:a:", long_options,
                                &option_index);
        
        /* Detect the end of the options */
        if (-1 == parameter)
            break;
        
        switch (parameter) {
            /* Verbose level */
            case 'l':
                globals.verbose = 1;
                globals.verbose_level = atoi(optarg);
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'l' set"));
                if (0 >= atoi(optarg)) {
                    OPTTRAN_OUTPUT(("%s (%d)",
                                    _ERROR("invalid debug level: too low"),
                                    atoi(optarg)));
                    show_help();
                }
                if (10 < atoi(optarg)) {
                    OPTTRAN_OUTPUT(("%s (%d)",
                                    _ERROR("invalid debug level: too high"),
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

            /* Use opttran? */
            case 'a':
                globals.use_opttran = 1;
                globals.opttrandir = optarg;
                OPTTRAN_OUTPUT_VERBOSE((10, "option 'a' set [%s]",
                                        globals.opttrandir));
                break;

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
                
            /* Unknown option */
            case '?':
                show_help();
                
            default:
                exit(OPTTRAN_ERROR);
        }
    }
    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("CLI params")));
    return OPTTRAN_SUCCESS;
}

/* parse_metrics_file */
static int parse_metrics_file(void) {
    FILE *metrics_FP;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    char *metrics_file;

    if (NULL != globals.metrics_file) {
        metrics_file = globals.metrics_file;
    } else {
        metrics_file = METRICS_FILE;
    }

    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s (%s)", _BLUE("Reading metrics file"),
                            metrics_file));

    if (NULL == (metrics_FP = fopen(metrics_file, "r"))) {
        OPTTRAN_OUTPUT(("%s (%s)", _ERROR("error openning metrics file"),
                        metrics_file));
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
        while (NULL != fgets(buffer, sizeof buffer, metrics_FP)) {
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
        sql[strlen(sql)-2] = 0; // remove the last ',' and '\n'
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
        OPTTRAN_OUTPUT_VERBOSE((3, "using STDIN as default input for performance measurements"));
    } else {
        OPTTRAN_OUTPUT_VERBOSE((3, "using FILE (%s) as default input for performance measurements",
                                globals.inputfile));
    }
    
    /* For each line in the INPUT file... */
    OPTTRAN_OUTPUT_VERBOSE((7, "--- parsing input file"));

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, sizeof buffer, inputfile_p)) {
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
            item = malloc(sizeof(segment_t));
            opttran_list_item_construct((opttran_list_item_t *) item);
            
            /* Add this item to 'segments' */
            opttran_list_append(segments_p, (opttran_list_item_t *) item);

            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "INSERT INTO %s (code_filename) VALUES ('new_code-%d');\n",
                    globals.metrics_table, (int)getpid());
            bzero(sql, BUFFER_SIZE);
            strcat(sql, temp_str);
            strcat(sql, "                           ");
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "SELECT id FROM %s WHERE code_filename = 'new_code-%d';",
                    globals.metrics_table, (int)getpid());
            strcat(sql, temp_str);
            
            OPTTRAN_OUTPUT_VERBOSE((5, "        SQL: %s", sql));
            
            /* Insert new code fragment into metrics database, retrieve id */
            if (SQLITE_OK != sqlite3_exec(globals.db, sql, get_rowid,
                                          (void *)&rowid, &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                sqlite3_close(globals.db);
                exit(OPTTRAN_ERROR);
            } else {
                OPTTRAN_OUTPUT_VERBOSE((5, "             ID: %d",
                                        rowid));
            }
            continue;
        }

        node = malloc(sizeof(node_t) + strlen(buffer) + 1);
        bzero(node, sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments' (only code.* parameters) other parameter should fit into
         * the SQL DB.
         */

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            item->filename = malloc(strlen(node->value) + 1);
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
            item->type = malloc(strlen(node->value) + 1);
            bzero(item->type, strlen(node->value) + 1);
            strcpy(item->type, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) type:         [%s]",
                                    input_line, item->type));
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            item->extra_info = malloc(strlen(node->value) + 1);
            bzero(item->extra_info, strlen(node->value) + 1);
            strcpy(item->extra_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) extra info:   [%s]",
                                    input_line, item->extra_info));
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
            item->section_info = malloc(strlen(node->value) + 1);
            bzero(item->section_info, strlen(node->value) + 1);
            strcpy(item->section_info, node->value);
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) section info: [%s]",
                                    input_line, item->section_info));
            free(node);
            continue;
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
        bzero(sql, 1024);
        sprintf(sql, "UPDATE %s SET %s='%s' WHERE id=%d;",
                globals.metrics_table, node->key, node->value, rowid);

        /* Update metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL,
                                      (void *)&rowid, &error_msg)) {
            OPTTRAN_OUTPUT_VERBOSE((4, "(%d) %s", input_line,
                                    _RED("ignored line")));
            sqlite3_free(error_msg);
        } else {
            OPTTRAN_OUTPUT_VERBOSE((10, "(%d) %s", input_line,
                                    sql));
        }
        free(node);
    }

    /* print a summary of 'segments' */
    OPTTRAN_OUTPUT_VERBOSE((4, "%d %s", opttran_list_get_size(segments_p),
                            _YELLOW("code segments found")));

    item = (segment_t *)opttran_list_get_first(segments_p);
    do {
        OPTTRAN_OUTPUT_VERBOSE((4, "   %s:%d", item->filename,
                                item->line_number));
        item = (segment_t *)opttran_list_get_next(item);
    } while ((opttran_list_item_t *)item != &(segments_p->sentinel));
    
    OPTTRAN_OUTPUT_VERBOSE((4, "==="));
    return OPTTRAN_SUCCESS;
}

/* get_rowid */
static int get_rowid(void *rowid, int col_count,
                                  char **col_values, char **col_names) {
    int *temp = (int *)rowid;
    *temp = atoi(col_values[0]);
    return OPTTRAN_SUCCESS;
}

/* output_recommendations */
static int output_recommendations(void *not_used, int col_count,
                                  char **col_values, char **col_names){
    
    OPTTRAN_OUTPUT_VERBOSE((7, "%s", _GREEN("new recommendation found")));

    /* If we are not plannning to use OPTTRAN, pint pretty, other just a list */
    if (0 == globals.use_opttran) {
        fprintf(globals.outputfile_FP, "# This is a possible recommendation for this code segment\n");
        fprintf(globals.outputfile_FP, "Recommendation: %s\n", col_values[0]);
        fprintf(globals.outputfile_FP, "Reason: %s\n", col_values[1]);
        fprintf(globals.outputfile_FP, "Transformation: %s\n", col_values[2] ? col_values[2] :
                "found no automatic transformation for this code segment");
        fprintf(globals.outputfile_FP, "Code example:\n%s\n", col_values[3]);
    } else {
        if (NULL != col_values[2]) {
            fprintf(globals.outputfile_FP, "%s\n", col_values[2]);
        }
    }

    return 0;
}

/* database_connect */
static int database_connect(void) {
    OPTTRAN_OUTPUT_VERBOSE((4, "=== %s", _BLUE("Connecting to database")));

    /* Connect to the DB */
    if (NULL == globals.dbfile) {
        globals.dbfile = "./recommendation.db";
    }
    if (-1 == access(globals.dbfile, F_OK)) {
        OPTTRAN_OUTPUT(("%s (%s) %s", _ERROR("recommendation database"),
                        globals.dbfile, _ERROR("doesn't exist")));
        return OPTTRAN_ERROR;
    }
    if (-1 == access(globals.dbfile, R_OK)) {
        OPTTRAN_OUTPUT(("%s (%s)",
                        _ERROR("you don't have permission to read the file"),
                        globals.dbfile));
        return OPTTRAN_ERROR;
    }
    
    if (SQLITE_OK != sqlite3_open(globals.dbfile, &(globals.db))) {
        OPTTRAN_OUTPUT(("%s (%s), %s", _ERROR("error openning database"),
                        globals.dbfile,
                        sqlite3_errmsg(globals.db)));
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    } else {
        OPTTRAN_OUTPUT_VERBOSE((4, "connected to %s", globals.dbfile));
    }
    return OPTTRAN_SUCCESS;
}

/* database_query */
static int database_query(void) {
    char *error_msg = NULL;

    OPTTRAN_OUTPUT_VERBOSE((7, "=== %s", _BLUE("Querying recommendation DB")));

    /* Query database */
    // TODO: put the right SQL query
    if (SQLITE_OK != sqlite3_exec(globals.db, "SELECT desc, reason, pattern, code FROM recommendation WHERE attr_code = 770 OR attr_code = 512 OR attr_code = 256 OR attr_code = 2;",
                                  output_recommendations, NULL, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        sqlite3_close(globals.db);
        exit(OPTTRAN_ERROR);
    }

    return OPTTRAN_SUCCESS;
}

/* calculate_weigths */
static int calculate_weigths(void) {
    // TODO: everything
    return OPTTRAN_SUCCESS;
}

/* select_recommendations */
static int select_recommendations(void) {
    database_query();
    // TODO: everything
    return OPTTRAN_SUCCESS;
}

// EOF
