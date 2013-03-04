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

#ifndef RECOMMENDER_H_
#define RECOMMENDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INSTALL_DIRS_H
#include "install_dirs.h"
#endif

#ifndef OPTTRAN_CONSTANTS_H_
#include "opttran_constants.h"
#endif

#ifndef OPTTRAN_LIST_H_
#include "opttran_list.h"
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif

#ifndef _GETOPT_H_
#include <getopt.h> /* To parse command line arguments */
#endif

#ifndef	_CTYPE_H_
#include <ctype.h>
#endif

#ifndef	_STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

/** Buffers size, will be used for:
 * - parsing INPUT file
 * - parsing metrics file
 * - storing SQL statements (including the 'functions')
 * - maybe something else
 */
#define BUFFER_SIZE 4096

/** Default values for some parameters */
#define RECOMMENDATION_DB       "recommendation.db"
#define METRICS_TABLE           "metric"
#define METRICS_FILE            "recommender-metrics.txt"
#define OPTTRAN_RECO_FILE       "recommendations.txt"
#define OPTTRAN_FRAGMENTS_DIR   "fragments"
#define OPTTRAN_SOURCE_DIR      "source"

/** Structure to hold global variables */
typedef struct {
    int  verbose;
    int  verbose_level;
    int  use_stdin;
    int  use_stdout;
    int  use_opttran;
    char *inputfile;
    char *outputfile;
    FILE *outputfile_FP;
    char *dbfile;
    sqlite3 *db;
    char *opttrandir;
    char *metrics_file;
    int  use_temp_metrics;
    char *metrics_table;
    int  colorful;
    char *source_file;
    int  rec_count;
    char *fragments_dir;
    unsigned long long int opttran_pid;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include opttran_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[recommender]"
    
#ifndef OPTTRAN_OUTPUT_H
#include "opttran_output.h"
#endif

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"verbose_level",   required_argument, NULL, 'l'},
    {"stdin",           no_argument,       NULL, 'i'},
    {"inputfile",       required_argument, NULL, 'f'},
    {"help",            no_argument,       NULL, 'h'},
    {"database",        required_argument, NULL, 'd'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"opttran",         required_argument, NULL, 'a'},
    {"recommendations", required_argument, NULL, 'r'},
    {"metricfile",      required_argument, NULL, 'm'},
    {"newmetrics",      no_argument,       NULL, 'n'},
    {"colorful",        no_argument,       NULL, 'c'},
    {"sourcefile",      required_argument, NULL, 's'},
    {"opttranid",       required_argument, NULL, 'p'},
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold recommendation selecting functions */
typedef struct function {
    volatile opttran_list_item_t *next;
    volatile opttran_list_item_t *prev;
    int id;
    char *desc;
    char statement[BUFFER_SIZE];
} function_t;

/** Structure to hold code segments */
typedef struct segment {
    volatile opttran_list_item_t *next; /** Pointer to next list item */
    volatile opttran_list_item_t *prev; /** Pointer to previous list item */
    char   *filename;
    int    line_number;
    char   *type;
    char   *extra_info;
    char   *section_info;
    double loop_depth;
    double representativeness;
    int    rowid;
    opttran_list_t functions;
    int    outer_loop;
    int    outer_outer_loop;
    char   *function_name; // Should add functiona_name to DB?
} segment_t;

/* Function declarations */
int recommender_main(int argc, char** argv);
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_metrics_file(void);
static int  parse_segment_params(opttran_list_t *segments_p, FILE *inputfile_p);
static int  get_rowid(void *rowid, int col_count, char **col_values,
                      char **col_names);
static int  get_weight(void *weight, int col_count, char **col_values,
                       char **col_names);
static int  output_patterns(void *weight, int col_count, char **col_values,
                            char **col_names);
static int  output_recommendations(void *not_used, int col_count,
                                   char **col_values, char **col_names);
static int  database_connect(void);
static int  accumulate_functions(void *functions, int col_count,
                                 char **col_values, char **col_names);
static int  select_recommendations(segment_t *segment);

#if HAVE_ROSE == 1
int open_rose(void);
int close_rose(void);
int extract_fragment(segment_t *segment);
int extract_source(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
