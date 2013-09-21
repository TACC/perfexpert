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

#ifndef _SQLITE3_H_
#include <sqlite3.h> /* To sqlite3 type on globals */
#endif

#ifndef	_STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

#include "perfexpert_list.h" /* To list items on function and segment */
#include "perfexpert_constants.h"

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[recommender]"
    
/** Structure to hold global variables */
typedef struct {
    FILE *inputfile_FP;
    FILE *outputfile_FP;
    FILE *outputmetrics_FP;
    char *inputfile;
    char *outputfile;
    char *outputmetrics;
    char *dbfile;
    char *workdir;
    char *metrics_file;
    char *metrics_table;
    int  verbose;
    int  colorful;
    int  rec_count;
    long int pid;
    sqlite3 *db;
} globals_t;

extern globals_t globals; /* This variable id declared in recommender_main.c */

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
    double importance;
    double runtime;
    int    loopdepth;
    int    rowid;
    perfexpert_list_t functions;
    char   *function_name; // Should I add function_name to DB?
} segment_t;

/* Function declarations */
void show_help(void);
int parse_env_vars(void);
int parse_cli_params(int argc, char *argv[]);
int parse_metrics_file(void);
int parse_segment_params(perfexpert_list_t *segments_p);
int output_recommendations(void *var, int count, char **val, char **names);
int accumulate_functions(void *functions, int count, char **val, char **names);
int select_recommendations(segment_t *segment);
int log_bottleneck(void *var, int count, char **val, char **names);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
