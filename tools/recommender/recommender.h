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

#ifndef RECOMMENDER_H_
#define RECOMMENDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INSTALL_DIRS_H
#include "install_dirs.h"
#endif

#ifndef PERFEXPERT_CONSTANTS_H_
#include "perfexpert_constants.h"
#endif

#ifndef PERFEXPERT_LIST_H_
#include "perfexpert_list.h"
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

/** Structure to hold global variables */
typedef struct {
    sqlite3 *db;
    int  verbose_level;
    char *inputfile;
    FILE *inputfile_FP;
    char *outputfile;
    FILE *outputfile_FP;
    char *dbfile;
    char *workdir;
    char *metrics_file;
    int  use_temp_metrics;
    char *metrics_table;
    int  colorful;
    int  rec_count;
    unsigned long long int perfexpert_pid;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[recommender]"
    
#ifndef PERFEXPERT_OUTPUT_H
#include "perfexpert_output.h"
#endif

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"automatic",       required_argument, NULL, 'a'},
    {"colorful",        no_argument,       NULL, 'c'},
    {"database",        required_argument, NULL, 'd'},
    {"inputfile",       required_argument, NULL, 'f'},
    {"help",            no_argument,       NULL, 'h'},
    {"verbose_level",   required_argument, NULL, 'l'},
    {"metricsfile",     required_argument, NULL, 'm'},
    {"newmetrics",      no_argument,       NULL, 'n'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"perfexpert_pid",  required_argument, NULL, 'p'},
    {"recommendations", required_argument, NULL, 'r'},
    {"sourcefile",      required_argument, NULL, 's'},
    {"verbose",         no_argument,       NULL, 'v'},
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold recommendation selecting functions */
typedef struct function {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    char *desc;
    char statement[BUFFER_SIZE];
} function_t;

/** Structure to hold code segments */
typedef struct segment {
    volatile perfexpert_list_item_t *next; /** Pointer to next list item */
    volatile perfexpert_list_item_t *prev; /** Pointer to previous list item */
    char   *filename;
    int    line_number;
    char   *type;
    char   *extra_info;
    char   *section_info;
    double loop_depth;
    double representativeness;
    int    rowid;
    perfexpert_list_t functions;
    char   *function_name; // Should add function_name to DB?
} segment_t;

/* Function declarations */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_metrics_file(void);
static int  parse_segment_params(perfexpert_list_t *segments_p);
static int  output_recommendations(void *var, int count, char **val, char **names);
static int  accumulate_functions(void *functions, int count, char **val, char **names);
static int  select_recommendations(segment_t *segment);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
