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

#ifndef PERFEXPERT_MODULE_LCPI_H_
#define PERFEXPERT_MODULE_LCPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "lcpi_types.h"
#include "modules/hpctoolkit/hpctoolkit_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_lcpi]"

/* Default values for some files and directories */
#define LCPI_VARIANCE_LIMIT 0.2

/* Module types */
typedef struct {
    perfexpert_list_t profiles;
    lcpi_metric_t *metrics_by_name;
    perfexpert_module_hpctoolkit_t *hpctoolkit;
    double threshold;
    char *order;
    int help_only;
    int mic;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);
int module_analyze(void);

/* Function declarations */
int parse_module_args(int argc, char *argv[]);
int metrics_generate(void);
int metrics_generate_mic(void);
int metrics_attach_machine(void);
int database_import(perfexpert_list_t *profiles, const char *table);
int database_export(perfexpert_list_t *profiles);
double database_get_hound(const char *name);
double database_get_event(const char *name, const char *table, int hotspot_id);
int logic_lcpi_compute(lcpi_profile_t *profile);
int output_analysis(perfexpert_list_t *profiles);
int hotspot_sort(perfexpert_list_t *profiles);

/* lcpi_get_value */
static inline double lcpi_get_value(lcpi_metric_t *db, const char *key) {
    lcpi_metric_t *entry = NULL;

    perfexpert_hash_find_str(db, perfexpert_md5_string(key), entry);

    if (NULL == entry) {
        return 0.0;
    } else {
        return entry->value;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_H_ */
