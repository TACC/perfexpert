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

#ifndef PERFEXPERT_MODULE_MACVEC_H_
#define PERFEXPERT_MODULE_MACVEC_H_

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
#include "macvec_types.h"
#include "modules/perfexpert_module_measurement.h"

/* Default values for some files and directories */
#define MACVEC_VARIANCE_LIMIT 0.2
#define MAX_MACVEC 1024

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_macvec]"

/* PerfExpert common headers */
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_string.h"

/* Module types */
typedef struct {
    perfexpert_list_t profiles;
    perfexpert_module_measurement_t *measurement;
    //char *architecture;
    int verbose;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);
int module_analyze(void);

/* Function declarations */
int database_import(perfexpert_list_t *profiles, char *filename);
int list_files_hotspots(perfexpert_list_t *files);

/* Processor specific functions */
int counters_jaketown(void);
int counters_mic(void);
int counters_papi(void); // Fallback for unknown processors

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

#endif /* PERFEXPERT_MODULE_MACVEC_H_ */
