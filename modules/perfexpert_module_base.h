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

#ifndef PERFEXPERT_BASE_MODULE_H_
#define PERFEXPERT_BASE_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef LTDL_H
#include <ltdl.h>
#endif

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"

/* Module constants */
#define PERFEXPERT_MODULE_NOT_IMPLEMENTED NULL

/* Module globals */
typedef struct {
    perfexpert_list_t modules;
    perfexpert_list_t compile;
    perfexpert_list_t measurements;
    perfexpert_list_t analysis;
} module_globals_t;

extern module_globals_t module_globals;

/* Module status */
typedef enum {
    PERFEXPERT_MODULE_LOADED = 0,
    PERFEXPERT_MODULE_INITIALIZED,
    PERFEXPERT_MODULE_FINALIZED,
} module_status_t;

/* Module order */
typedef enum {
    PERFEXPERT_MODULE_BEFORE = 0,
    PERFEXPERT_MODULE_AFTER,
    PERFEXPERT_MODULE_AVAILABLE,
} module_order_t;

/* Module phase */
typedef enum {
    PERFEXPERT_MODULE_COMPILE = 0,
    PERFEXPERT_MODULE_MEASUREMENTS,
    PERFEXPERT_MODULE_ANALYSIS,
    PERFEXPERT_MODULE_ALL,
} module_phase_t;

/* Module interface */
typedef int (*perfexpert_module_load_fn_t)(void);
typedef int (*perfexpert_module_init_fn_t)(void);
typedef int (*perfexpert_module_fini_fn_t)(void);
typedef int (*perfexpert_module_compile_fn_t)(void);
typedef int (*perfexpert_module_measurements_fn_t)(void);
typedef int (*perfexpert_module_analysis_fn_t)(void);

/* Base module structure. Any module should implement these functions */
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char *version;
    int  argc;
    char *argv[MAX_ARGUMENTS_COUNT];
    module_status_t status;
    perfexpert_module_load_fn_t load;
    perfexpert_module_init_fn_t init;
    perfexpert_module_fini_fn_t fini;
    perfexpert_module_compile_fn_t compile;
    perfexpert_module_measurements_fn_t measurements;
    perfexpert_module_analysis_fn_t analysis;
    /* Extended module interface from this point */
} perfexpert_module_1_0_0_t;

typedef perfexpert_module_1_0_0_t perfexpert_module_t;

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    perfexpert_module_t *module;
} perfexpert_ordered_module_t;

/* Function declaration */
void __attribute__ ((constructor)) my_init(void);
int perfexpert_module_load(const char *name);
int perfexpert_module_set_option(const char *module, const char *option);
int perfexpert_module_init(void);
int perfexpert_module_fini(void);
int perfexpert_module_compile(void);
int perfexpert_module_measurements(void);
int perfexpert_module_analysis(void);
int perfexpert_module_requires(const char *a, const char *b,
    module_order_t order, module_phase_t phase);
static lt_dlhandle perfexpert_module_open(const char *name);
perfexpert_module_t *perfexpert_module_available(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_BASE_MODULE_H_ */
