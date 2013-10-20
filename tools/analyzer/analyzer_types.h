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

#ifndef ANALYZER_TYPES_H_
#define ANALYZER_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H__
#include <stdio.h>
#endif

#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_list.h"

/* Structure to hold lcpi metrics */
typedef struct lcpi {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char name_md5[33];
    double value;
    perfexpert_hash_handle_t hh_str;
    /* From this point this structure is different than metric_t */
    void *expression;
} lcpi_t;

/* Structure to hold modules */
typedef struct module {
    int  id;
    char *name;
    char *shortname;
    double importance;
    double cycles;
    perfexpert_hash_handle_t hh_int;
} module_t;

/* Structure to hold files */
typedef struct file {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh_int;
} file_t;

/* Structure to hold metrics and machine characterization */
typedef struct metric {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char   *name;
    char   name_md5[33];
    double value;
    perfexpert_hash_handle_t hh_str;
    /* From this point this structure is different than lcpi_t */
    int id;
    int thread;
    int mpi_rank;
    int experiment;
    perfexpert_hash_handle_t hh_int;
} metric_t;

/* Structure to hold procedures */
typedef struct procedure {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *name;
    int  line;
    int  valid;
    double variance;
    double importance;
    double instructions;
    double cycles;
    enum hotspot_type_t type;
    lcpi_t *lcpi_by_name;
    metric_t *metrics_by_id;
    metric_t *metrics_by_name;
    perfexpert_list_t metrics;
    /* From this point this struct is different than loop_t */
    perfexpert_hash_handle_t hh_int;
    module_t *module;
    file_t *file;
} procedure_t;

/* Structure to hold loops */
typedef struct loop {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *name;
    int  line;
    int  valid;
    double variance;
    double importance;
    double instructions;
    double cycles;
    enum hotspot_type_t type;
    lcpi_t *lcpi_by_name;
    metric_t *metrics_by_id;
    metric_t *metrics_by_name;
    perfexpert_list_t metrics;
    /* From this point this struct is different than procedure_t */
    procedure_t *procedure;
    int  depth;
} loop_t;

/* Structure to hold the call path */
typedef struct callpath callpath_t;
struct callpath {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t callees;
    int id;
    int scope;
    int alien;
    callpath_t *parent;
    procedure_t *procedure;
};

/* Structure to hold profiles */
typedef struct profile {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t callees;
    int  id;
    char *name;
    double cycles;
    double instructions;
    file_t *files_by_id;
    module_t *modules_by_id;
    metric_t *metrics_by_id;
    metric_t *metrics_by_name;
    procedure_t *procedures_by_id;
    perfexpert_list_t hotspots; /* for both procedure_t and loop_t */
} profile_t;

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_H_ */
