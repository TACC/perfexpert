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

#ifndef PERFEXPERT_H_
#define PERFEXPERT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert]"

/* Structure to hold global variables */
typedef struct {
    int   verbose;
    char  *dbfile;
    int   colorful;
    float threshold;
    int   rec_count;
    int   leave_garbage;
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
    char  *tool;
    long int pid;
} globals_t;

extern globals_t globals; /* This variable is declared in perfexpert_main.c */

/* Function declarations */
void show_help(void);
int parse_env_vars(void);
int parse_cli_params(int argc, char *argv[]);
int compile_program(void);
int measurements(void);
int analysis(void);
int recommendation(void);
int transformation(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_H */
