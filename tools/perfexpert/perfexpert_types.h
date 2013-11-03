/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_TYPES_H_
#define PERFEXPERT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h> /* To sqlite3 type on globals */
#endif

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* Structure to hold global variables */
typedef struct {
    int  verbose;
    int  colorful; // These should be the first variables in the structure
    double threshold;
    char *dbfile;
    int  rec_count;
    int  leave_garbage;
    char *target;
    char *sourcefile;
    char *program;
    char *program_path;
    char *program_full;
    char *program_argv[MAX_ARGUMENTS_COUNT];
    int  step;
    char *workdir;
    char *stepdir;
    char *prefix[MAX_ARGUMENTS_COUNT];
    char *before[MAX_ARGUMENTS_COUNT];
    char *after[MAX_ARGUMENTS_COUNT];
    char *knc;
    char *knc_prefix[MAX_ARGUMENTS_COUNT];
    char *knc_before[MAX_ARGUMENTS_COUNT];
    char *knc_after[MAX_ARGUMENTS_COUNT];
    char *order;
    char *inputfile;
    int  only_exp;
    int  compat_mode;
    long int pid;
    long long int unique_id;
    sqlite3 *db;
} globals_t;

extern globals_t globals; /* This variable is declared in perfexpert_main.c */

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_TYPES_H */
