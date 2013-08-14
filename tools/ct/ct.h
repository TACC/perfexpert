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

#ifndef CT_H_
#define CT_H_

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

#ifndef _GETOPT_H
#include <getopt.h> /* To parse command line arguments */
#endif

#ifndef	_STDIO_H
#include <stdio.h> /* To use FILE type on globals */
#endif

/** Structure to hold global variables */
typedef struct {
    int      verbose_level;
    char     *inputfile;
    FILE     *inputfile_FP;
    char     *outputfile;
    FILE     *outputfile_FP;
    int      colorful;
    char     *workdir;
    char     *dbfile;
    sqlite3  *db;
    long int pid;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert:ct]"

#ifndef PERFEXPERT_OUTPUT_H
#include "perfexpert_output.h"
#endif

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"automatic",       required_argument, NULL, 'a'},
    {"database",        required_argument, NULL, 'd'},
    {"colorful",        no_argument,       NULL, 'c'},
    {"inputfile",       required_argument, NULL, 'f'},
    {"help",            no_argument,       NULL, 'h'},
    {"verbose_level",   required_argument, NULL, 'l'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"pid",             required_argument, NULL, 'p'},
    {"verbose",         no_argument,       NULL, 'v'},
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold patterns */
typedef struct pattern {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *program;
} pattern_t;

/** Structure to hold transformations */
typedef struct transformation {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *program;
    perfexpert_list_t patterns;
} transformation_t;

/** Structure to hold recommendations */
typedef struct recommendation {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    perfexpert_list_t transformations;
} recommendation_t;

/** Structure to hold fragments */
typedef struct fragment {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *filename;
    int  line_number;
    char *function_name;
    char *code_type;
    int  rowid;
    int  loop_depth;
    perfexpert_list_t recommendations;
    /* The fields below have local information */
    char *fragment_file;
    char *outer_loop_fragment_file;
    char *outer_outer_loop_fragment_file;
    int  outer_loop_line_number;
    int  outer_outer_loop_line_number;
} fragment_t;

/* Function declarations */
int ct_main(int argc, char** argv);
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_transformation_params(perfexpert_list_t *fragments);
static int  select_transformations(fragment_t *fragment);
static int  accumulate_transformations(void *recommendation,
    int count, char **val, char **names);
static int  accumulate_patterns(void *transformation, int count,
    char **val, char **names);
static int  apply_recommendations(fragment_t *fragment);
static int  apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation);
static int  apply_patterns(fragment_t *fragment,
    recommendation_t *recommendation, transformation_t *transformation);
static int  test_transformation(fragment_t *fragment,
    recommendation_t *recommendation, transformation_t *transformation);
static int  test_pattern(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation, pattern_t *pattern);

/* Rose functions */
int open_rose(const char *source_file);
int close_rose(void);
int extract_fragment(fragment_t *fragment);
int extract_source(void);

#ifdef __cplusplus
}
#endif

#endif /* CT_H */
