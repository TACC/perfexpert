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

#ifndef ANALYZER_H_
#define ANALYZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H__
#include <stdio.h>
#endif

#ifndef __XML_PARSER_H__
#include <libxml/parser.h>
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

/* Structure to hold global variables */
typedef struct {
    double threshold;
    char     *tool;
    char     *inputfile;
    char     *outputfile;
    FILE     *outputfile_FP;
    int      aggregate;
    int      thread;
    char     *machinefile;
    metric_t *machine_by_name;
    char     *lcpifile;
    lcpi_t   *lcpi_by_name;
    int      verbose;
    int      colorful;
    char     *outputmetrics;
    char     *order;
} globals_t;

extern globals_t globals; /* This variable is defined in analyzer_main.c */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[analyzer]"

/* Function declarations */
void show_help(void);
int parse_env_vars(void);
int parse_cli_params(int argc, char *argv[]);
int profile_parse_file(const char* file, const char* tool,
    perfexpert_list_t *profiles);
int profile_check_all(perfexpert_list_t *profiles);
int profile_check_callpath(perfexpert_list_t *calls, int root);
int profile_flatten_all(perfexpert_list_t *profiles);
int profile_flatten_hotspots(profile_t *profile);
int profile_aggregate_hotspots(profile_t *profile);
int profile_aggregate_metrics(profile_t *profile, procedure_t *hotspot);
int lcpi_parse_file(const char *file);
int lcpi_compute(profile_t *profile);
int machine_parse_file(const char *file);
int calculate_importance_variance(profile_t *profile);
int output_analysis_all(perfexpert_list_t *profiles);
int output_analysis(profile_t *profile, procedure_t *hotspot);
int output_metrics_all(perfexpert_list_t *profiles);
int output_metrics(profile_t *profile, procedure_t *hotspot, FILE *file_FP);
int hotspot_sort(perfexpert_list_t *profiles);

/* generic_get */
#include "perfexpert_alloc.h"
#include "perfexpert_md5.h"
static inline double generic_get(lcpi_t *db, char key[]) {
    lcpi_t *entry = NULL;
    char *key_md5 = NULL;
    double value;

    PERFEXPERT_ALLOC(char, key_md5, 33);
    strcpy(key_md5, perfexpert_md5_string(key));
    perfexpert_hash_find_str(db, key_md5, entry);
    PERFEXPERT_DEALLOC(key_md5);

    if (NULL == entry) {
        return 0.0;
    } else {
        return entry->value;
    }
}

/* Convenience macros to get data from machine_t and lcpi_t */
#define perfexpert_lcpi_definition_get(_1) generic_get(globals.lcpi_by_name, _1)
#define perfexpert_lcpi_hotspot_get(_1, _2) generic_get(_1->lcpi_by_name, _2)
#define perfexpert_machine_get(_1) \
    generic_get((lcpi_t *)globals.machine_by_name, _1)

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_H_ */
