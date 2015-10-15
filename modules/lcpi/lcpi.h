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

#ifndef _I_AM_A_MODULE_
#define _I_AM_A_MODULE_
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>

/* Utility headers */
#include <matheval.h>

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "lcpi_types.h"
#include "modules/perfexpert_module_measurement.h"

/* Default values for some files and directories */
#define LCPI_VARIANCE_LIMIT 0.2
#define MAX_LCPI 1024

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_lcpi]"

/* Module types */
typedef struct {
    perfexpert_list_t profiles;
    lcpi_metric_t *metrics_by_name;
    perfexpert_module_measurement_t *measurement;
    double threshold;
    char *order;
    int help_only;
    int mic;
    char *architecture;
    int verbose;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* PerfExpert common headers */
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_string.h"

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);
int module_analyze(void);

/* Function declarations */
int parse_module_args(int argc, char *argv[]);
int metrics_papi_generate_mic(void);
int metrics_intel_generate(void);
int metrics_intel_generate_mic(void);
int metrics_attach_machine(void);
int database_import(perfexpert_list_t *profiles, const char *table);
int database_export(perfexpert_list_t *profiles, const char *table);
double database_get_hound(const char *name);
double database_get_event(const char *name, const char *table, int hotspot_id);
int logic_lcpi_compute(lcpi_profile_t *profile);
int output_analysis(perfexpert_list_t *profiles);
int hotspot_sort(perfexpert_list_t *profiles);

/* Processor specific functions */
int metrics_jaketown(void);
int metrics_jaketown_vtune(void);
int metrics_mic(void);
int metrics_papi(void); // Fallback for unknown processors

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

/* lcpi_add_metric */
static inline int lcpi_add_metric (char *name, char *value) {
    lcpi_metric_t *metric;

    /* Replace the '.' on the metric, GNU libmatheval does not like them... */
    perfexpert_string_replace_char(value, '.', '_');

    PERFEXPERT_ALLOC(lcpi_metric_t, metric, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, metric->name, (strlen(name) + 1));
    strcpy(metric->name, name);
    strcpy(metric->name_md5, perfexpert_md5_string(metric->name));
    metric->value = 0.0;
    metric->expression = evaluator_create(value);
    if (NULL == metric->expression) {
        OUTPUT(("%s (%s) [%s]", _ERROR("invalid expression"), name, value));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, metric);

    return PERFEXPERT_SUCCESS;
}

#define USE_EVENT(X)                                                           \
    if (NULL == my_module_globals.measurement->set_event) {                    \
        OUTPUT(("%s [%s]", _ERROR("'set_event' function not found on module"), \
            my_module_globals.measurement->name));                             \
        return PERFEXPERT_ERROR;                                               \
    }                                                                          \
    if (PERFEXPERT_SUCCESS != my_module_globals.measurement->set_event(X)) {   \
        OUTPUT(("%s [%s]", _ERROR("invalid event name"), X));                  \
        return PERFEXPERT_ERROR;                                               \
    }

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_H_ */
