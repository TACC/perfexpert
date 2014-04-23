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

#ifndef PREFEXPERT_MODULE_MAKE_MODULE_H_
#define PREFEXPERT_MODULE_MAKE_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "modules/perfexpert_module_base.h"

/* Interface extenstions */
typedef int (*perfexpert_module_make_set_target_fn_t)(const char *target);
typedef int (*perfexpert_module_make_set_env_fn_t)(const char *name,
    const char *value);
typedef int (*perfexpert_module_make_unset_env_fn_t)(const char *name);
typedef char* (*perfexpert_module_make_get_env_fn_t)(const char *name);

/* Make module interface */
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
    perfexpert_module_make_set_target_fn_t set_target;
    perfexpert_module_make_set_env_fn_t set_env;
    perfexpert_module_make_unset_env_fn_t unset_env;
    perfexpert_module_make_get_env_fn_t get_env;
} perfexpert_module_make_1_0_0_t;

typedef perfexpert_module_make_1_0_0_t perfexpert_module_make_t;

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_MAKE_MODULE_H_ */
