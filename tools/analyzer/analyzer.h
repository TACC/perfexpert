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

#ifndef __XML_TREE_H__
#include <libxml/tree.h>
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

#define PERFEXPERT_FUNCTION 1
#define PERFEXPERT_LOOP     2

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
    perfexpert_hash_handle_t hh;
} module_t;

/** Structure to hold files */
typedef struct file {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh;
} file_t;

/** Structure to hold procedures */
typedef struct procedure {
    int  id;
    char *name;
    perfexpert_hash_handle_t hh;
} procedure_t;

/** Structure to hold metrics */
typedef struct metric {
    int    id;
    char   *name;
    int    thread;
    int    sample;
    double value;
    perfexpert_hash_handle_t hh;
} metric_t;

/** Structure to hold the call path */
typedef struct callpath callpath_t;
struct callpath {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    int  line;
    int  scope;
    int  alien;
    int  type;
    file_t *file;
    module_t *module;
    metric_t *metrics;
    callpath_t *parent;
    procedure_t *procedure;
    perfexpert_list_t calls;
    perfexpert_hash_handle_t hh;
};

/** Structure to hold profiles */
typedef struct profile {
    int  id;
    char *name;
    metric_t *metrics;
    procedure_t *procedures;
    file_t *files;
    module_t *modules;
    callpath_t *callpaths;
    perfexpert_hash_handle_t hh;
} profile_t;

/* Function declarations */
int hpctoolkit_parse_file(const char *file, profile_t *profiles);
int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node, profile_t *profiles,
    callpath_t *callpath, callpath_t *parent);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_H */
