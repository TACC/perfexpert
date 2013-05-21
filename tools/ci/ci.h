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

#ifndef CI_H_
#define CI_H_

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
    char *outputdir;
    int  colorful;
    int  automatic;
    char *workdir;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert:ci]"

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
    {"colorful",        no_argument,       NULL, 'c'},
    {"automatic",       required_argument, NULL, 'a'},
    {"outputdir",       required_argument, NULL, 'o'},
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold function/code replacements */
typedef struct function {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *source_file;
    char *function_name;
    char *replacement_file;
    int  line_number;
    void *node;
} function_t;

/* Function declarations */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_transformation_params(perfexpert_list_t *segments_p,
                                        FILE *inputfile_p);
int open_rose(char *source_file);
int close_rose(void);
int replace_function(function_t *function);

#ifdef __cplusplus
}
#endif

#endif /* CI_H */
