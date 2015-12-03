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

#ifndef PREFEXPERT_MODULE_MEASUREMENT_H_
#define PREFEXPERT_MODULE_MEASUREMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "modules/perfexpert_module_base.h"

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* Interface extenstions */
typedef int (*perfexpert_module_measurement_set_event_fn_t)(const char *name);

/* HPCToolkit module interface */
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char *version;
    int  argc;
    char *argv[MAX_ARGUMENTS_COUNT];
    int  verbose_level;
    perfexpert_module_status_t status;
    perfexpert_module_load_fn_t load;
    perfexpert_module_init_fn_t init;
    perfexpert_module_fini_fn_t fini;
    perfexpert_module_compile_fn_t compile;
    perfexpert_module_instrument_fn_t instrument;
    perfexpert_module_measure_fn_t measure;
    perfexpert_module_analyze_fn_t analyze;
    perfexpert_module_recommend_fn_t recommend;
    /* Extended module interface */
    perfexpert_module_measurement_set_event_fn_t set_event;
    char total_cycles_counter[80];
    char total_inst_counter[80];
} perfexpert_module_measurement_1_0_0_t;

typedef perfexpert_module_measurement_1_0_0_t perfexpert_module_measurement_t;

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_MEASUREMENT_H_ */
