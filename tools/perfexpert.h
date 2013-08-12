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
    int   verbose_level;
    char  *dbfile;
    int   colorful;
    float threshold;
    int   rec_count;
    int   left_garbage;
    char  *target;
    char  *sourcefile;
    char  *program;
    int   prog_arg_pos;
    int   main_argc;
    char  **main_argv;
    char  *before;
    char  *after;
    int   step;
    char  *workdir;
    char  *stepdir;
    char  *prefix;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert]"

#ifndef PERFEXPERT_OUTPUT_H
#include "perfexpert_output.h"
#endif

/** Structure to handles command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"after",         required_argument, NULL, 'a'},
    {"before",        required_argument, NULL, 'b'},
    {"colorful",      no_argument,       NULL, 'c'},
    {"database",      required_argument, NULL, 'd'},
    {"left-garbage",  no_argument,       NULL, 'g'},
    {"help",          no_argument,       NULL, 'h'},
    {"verbose_level", required_argument, NULL, 'l'},
    {"makefile",      required_argument, NULL, 'm'},
    {"prefix",        required_argument, NULL, 'p'},
    {"recommend",     required_argument, NULL, 'r'},
    {"source",        required_argument, NULL, 's'},
    {"verbose",       no_argument,       NULL, 'v'},
    {0, 0, 0, 0}
};

/* Function declarations */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  compile_program(void);
static int  measurements(void);
static int  analysis(void);
static int  run_hpcstruct(void);
static int  run_hpcrun(void);
static int  run_hpcprof(void);
static int  recommendation(void);
static int  transformation(void);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
