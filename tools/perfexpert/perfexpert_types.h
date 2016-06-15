/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
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
#include "common/perfexpert_backup.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* Structure to hold global variables */
typedef struct {
    int  verbose;
    int  colorful;
    char *prefix;  // what to prepend before executing the command
    char *after;   // Code to run after the command (if any)
    char *before;  // Code to run before the command (if any)
    char *workdir; // These should be the first variables in the structure
    char *dbfile;
    int  remove_garbage;
    char *program;
    char *program_path;
    char *program_full;
    char *program_argv[MAX_ARGUMENTS_COUNT];
    int  cycle;
    char *moduledir;
    long long int unique_id;
    sqlite3 *db;
    perfexpert_backup_t backup;
    double threshold;
    int output_mode;
} globals_t;

extern globals_t globals; /* This variable is defined in perfexpert_main.c */

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_TYPES_H */
