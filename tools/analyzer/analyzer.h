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

#ifndef __XML_PARSER_H__
#include <libxml/parser.h>
#endif

// #include "uthash.h"
#include "perfexpert_hash.h"
#include "perfexpert_list.h"

/** Structure to hold global variables */
typedef struct {
    int  verbose_level;
    int  colorful;
} globals_t;

extern globals_t globals; /* This variable is defined in analyzer_main.c */

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[analyzer]"

#define PERFEXPERT_FUNCTION  0
#define PERFEXPERT_LOOP      1

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
// static struct option long_options[] = {
//     {"colorful",      no_argument,       NULL, 'c'},
//     {"inputfile",     required_argument, NULL, 'f'},
//     {"help",          no_argument,       NULL, 'h'},
//     {"verbose_level", required_argument, NULL, 'l'},
//     {"outputfile",    required_argument, NULL, 'o'},
//     {"verbose",       no_argument,       NULL, 'v'},
//     {0, 0, 0, 0}
// };

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold modules */
typedef struct module {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh_int;
} module_t;

/** Structure to hold files */
typedef struct file {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh_int;
} file_t;

/** Structure to hold procedures */
typedef struct procedure {
    int  id;
    char *name;
    perfexpert_hash_handle_t hh_int;
} procedure_t;

/** Structure to hold metrics */
typedef struct metric {
    int    id;
    char   *name;
    char   name_md5[33];
    int    thread;
    int    experiment;
    double value;
    perfexpert_hash_handle_t hh_int;
    perfexpert_hash_handle_t hh_str;
} metric_t;

/** Structure to hold the call path */
typedef struct callpath callpath_t;
struct callpath {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t callees;
    int  id;
    int  line;
    int  type;
    int  scope;
    int  alien;
    int  loopdepth;
    file_t *file;
    module_t *module;
    metric_t *metrics_by_id;
    metric_t *metrics_by_name;
    callpath_t *parent;
    procedure_t *procedure;
};

/** Structure to hold profiles */
typedef struct profile {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t callees;
    int  id;
    char *name;
    file_t *files;
    metric_t *metrics_by_id;
    metric_t *metrics_by_name;
    module_t *modules;
    procedure_t *procedures;
} profile_t;

/* Function declarations */
int hpctoolkit_parse_file(const char *file, perfexpert_list_t *profiles);
int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth);
int check_profiles(perfexpert_list_t *profiles);
int check_callpath(perfexpert_list_t *calls, int root);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_H */
