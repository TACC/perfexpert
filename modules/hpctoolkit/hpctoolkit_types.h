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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_TYPES_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert common headers */
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"

/* Module types */
typedef struct {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh_int;
} hpctoolkit_module_t;

typedef struct {
    int  id;
    char *name;
    char *shortname;
    perfexpert_hash_handle_t hh_int;
} hpctoolkit_file_t;

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char name_md5[33];
    int id;
    int thread;
    int mpi_rank;
    int experiment;
    double value;
    perfexpert_hash_handle_t hh_int;
} hpctoolkit_metric_t;

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *name;
    int  line;
    hotspot_type_t type;
    hpctoolkit_metric_t *metrics_by_id;
    perfexpert_list_t metrics;
    /* From this point this struct is different than loop_t */
    perfexpert_hash_handle_t hh_int;
    hpctoolkit_module_t *module;
    hpctoolkit_file_t *file;
} hpctoolkit_procedure_t;

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int  id;
    char *name;
    int  line;
    hotspot_type_t type;
    hpctoolkit_metric_t *metrics_by_id;
    perfexpert_list_t metrics;
    /* From this point this struct is different than procedure_t */
    hpctoolkit_procedure_t *procedure;
    int  depth;
} hpctoolkit_loop_t;

typedef struct hpctoolkit_callpath hpctoolkit_callpath_t;
struct hpctoolkit_callpath {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    hpctoolkit_callpath_t *parent;
    hpctoolkit_procedure_t *procedure;
    perfexpert_list_t callees;
};

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t callees;
    int  id;
    char *name;
    hpctoolkit_file_t *files_by_id;
    hpctoolkit_module_t *modules_by_id;
    hpctoolkit_metric_t *metrics_by_id;
    hpctoolkit_procedure_t *procedures_by_id;
    perfexpert_list_t hotspots; /* for both procedure_t and loop_t */
} hpctoolkit_profile_t;

typedef struct {
    char *name;
    char name_md5[33];
    perfexpert_hash_handle_t hh_str;
} hpctoolkit_event_t;

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_TYPES_H_ */
