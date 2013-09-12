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

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _GETOPT_H_
#include <getopt.h> /* To parse command line arguments */
#endif

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert]"

/** Structure to hold global variables */
typedef struct {
    int   verbose_level;
    char  *dbfile;
    int   colorful;
    float threshold;
    int   rec_count;
    int   clean_garbage;
    char  *target;
    char  *sourcefile;
    char  *program;
    char  *program_path;
    char  *program_full;
    int   prog_arg_pos;
    int   main_argc;
    char  **main_argv;
    int   step;
    char  *workdir;
    char  *stepdir;
    char  *prefix;
    char  *before;
    char  *after;
    char  *knc;
    char  *knc_prefix;
    char  *knc_before;
    char  *knc_after;
    long int pid;
} globals_t;

extern globals_t globals; /* This variable is declared in perfexpert_main.c */

/** Structure to handles command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"after",         required_argument, NULL, 'a'},
    {"knc-after",     required_argument, NULL, 'A'},
    {"before",        required_argument, NULL, 'b'},
    {"knc-before",    required_argument, NULL, 'B'},
    {"colorful",      no_argument,       NULL, 'c'},
    {"database",      required_argument, NULL, 'd'},
    {"clean-garbage", no_argument,       NULL, 'g'},
    {"help",          no_argument,       NULL, 'h'},
    {"knc",           required_argument, NULL, 'k'},
    {"verbose-level", required_argument, NULL, 'l'},
    {"makefile",      required_argument, NULL, 'm'},
    {"prefix",        required_argument, NULL, 'p'},
    {"knc-prefix",    required_argument, NULL, 'P'},
    {"recommend",     required_argument, NULL, 'r'},
    {"source",        required_argument, NULL, 's'},
    {"verbose",       no_argument,       NULL, 'v'},
    {0, 0, 0, 0}
};

/* Function declarations */
void show_help(void);
int parse_env_vars(void);
int parse_cli_params(int argc, char *argv[]);
int compile_program(void);
int measurements(void);
int analysis(void);
int run_hpcstruct(void);
int run_hpcrun(void);
int run_hpcrun_knc(void);
int run_hpcprof(void);
int recommendation(void);
int transformation(void);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
