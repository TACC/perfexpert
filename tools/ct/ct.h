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

#ifndef CT_H_
#define CT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif

#ifndef _STDIO_H
#include <stdio.h> /* To use FILE type on globals */
#endif

#include "perfexpert_constants.h"
#include "perfexpert_list.h"

/* Structure to hold global variables */
typedef struct {
    FILE *inputfile_FP;
    FILE *outputfile_FP;
    char *inputfile;
    char *outputfile;
    char *workdir;
    char *dbfile;
    int  verbose;
    int  colorful;
    long int pid;
    sqlite3 *db;
} globals_t;

extern globals_t globals; /* This variable is defined in ct_real_main.c */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert:ct]"

/* Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/* Structure to hold patterns */
typedef struct pattern {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *program;
} pattern_t;

/* Structure to hold transformations */
typedef struct transformation {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *program;
    perfexpert_list_t patterns;
} transformation_t;

/* Structure to hold recommendations */
typedef struct recommendation {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    perfexpert_list_t transformations;
} recommendation_t;

/* Structure to hold fragments */
typedef struct fragment {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *filename;
    int  line_number;
    char *function_name;
    int  rowid;
    int  loop_depth;
    enum hotspot_type_t code_type;
    perfexpert_list_t recommendations;
    /* The fields below have local information */
    char *fragment_file;
    char *outer_loop_fragment_file;
    char *outer_outer_loop_fragment_file;
    int  outer_loop_line_number;
    int  outer_outer_loop_line_number;
} fragment_t;

/* C function declarations */
int ct_main(int argc, char** argv);
void show_help(void);
int parse_env_vars(void);
int parse_cli_params(int argc, char *argv[]);
int parse_transformation_params(perfexpert_list_t *fragments);
int select_transformations(fragment_t *fragment);
int accumulate_transformations(void *recommendation, int count, char **val,
    char **names);
int accumulate_patterns(void *transformation, int count, char **val,
    char **names);
int apply_recommendations(fragment_t *fragment);
int apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation);
int apply_patterns(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation);
int test_transformation(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation);
int test_pattern(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation, pattern_t *pattern);

/* C++ function declarations (declared as C for linkage purposes) */
int open_rose(const char *source_file);
int close_rose(void);
int extract_fragment(fragment_t *fragment);

#ifdef __cplusplus
}
#endif

#endif /* CT_H */
