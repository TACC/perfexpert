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

#ifndef RECOMMENDER_H_
#define RECOMMENDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h> /* To sqlite3 type on globals */
#endif

#ifndef _STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[recommender]"

/* Structure to hold global variables */
typedef struct {
    FILE *outputfile_FP;
    FILE *outputmetrics_FP;
    char *outputfile;
    char *outputmetrics;
    char *dbfile;
    char *workdir;
    int  verbose;
    int  colorful;
    int  rec_count;
    int  no_rec;
    long long int uid;
    long int pid;
    sqlite3 *db;
    perfexpert_list_t strategies;
} globals_t;

extern globals_t globals; /* This variable id declared in recommender_main.c */

/* Function declarations */
int parse_cli_params(int argc, char *argv[]);
int select_recommendations(void);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_H */
