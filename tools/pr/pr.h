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

#ifndef PR_H_
#define PR_H_

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

#if HAVE_SQLITE3 == 1
#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif
#endif

#ifndef _GETOPT_H_
#include <getopt.h> /* To parse command line arguments */
#endif

#ifndef	_STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

/** Structure to hold global variables */
typedef struct {
    int  verbose;
    int  verbose_level;
    int  use_stdin;
    int  use_stdout;
    char *inputfile;
    char *outputfile;
    FILE *outputfile_FP;
    int  colorful;
    int  testall;
    int  use_opttran;
    char *opttrandir;
#if HAVE_SQLITE3 == 1
    char *dbfile;
    sqlite3 *db;
    unsigned long long int opttran_pid;
#endif
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include perfepxert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[pr]"

#ifndef PERFEXPERT_OUTPUT_H
#include "perfexpert_output.h"
#endif

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"verbose_level",   required_argument, NULL, 'l'},
    {"verbose",         no_argument,       NULL, 'v'},
    {"stdin",           no_argument,       NULL, 'i'},
    {"inputfile",       required_argument, NULL, 'f'},
    {"help",            no_argument,       NULL, 'h'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"colorful",        no_argument,       NULL, 'c'},
    {"testall",         no_argument,       NULL, 't'},
    {"opttran",         required_argument, NULL, 'a'},
#if HAVE_SQLITE3 == 1
    {"opttranid",       required_argument, NULL, 'p'},
#endif
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Ninja structure to hold a list of tests to perform */
typedef struct test {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *program;
    char *fragment_file;
    int  *test_result;
    int  fragment_id;
} test_t;

/** Structure to hold recognizers */
typedef struct recognizer {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *program;
    int  test_result;
    int  test2_result;
    int  test3_result;
} recognizer_t;

/** Structure to hold recommendations */
typedef struct recommendation {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    perfexpert_list_t recognizers;
} recommendation_t;

/** Structure to hold code transformation patterns */
typedef struct fragment {
    volatile perfexpert_list_item_t *next; /** Pointer to next list item */
    volatile perfexpert_list_item_t *prev; /** Pointer to previous list item */
    char   *filename;
    int    line_number;
    char   *code_type;
    int    loop_depth;
    char   *fragment_file;
    int    outer_loop;
    char   *outer_loop_fragment_file;
    int    outer_outer_loop;
    char   *outer_outer_loop_fragment_file;
    char   *function_name;
    perfexpert_list_t recommendations;
} fragment_t;

/* Function declarations */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_fragment_params(perfexpert_list_t *segments_p,
                                  FILE *inputfile_p);
static int  test_recognizers(perfexpert_list_t *fragments_p);
static int  test_one(test_t *test);
static int  output_results(perfexpert_list_t *fragments_p);
#if HAVE_SQLITE3 == 1
static int  database_connect(void);
#endif
    
#ifdef __cplusplus
}
#endif

#endif /* PR_H */
