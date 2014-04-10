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

/* Module globals */
typedef struct {
    perfexpert_list_t modules;
    perfexpert_list_t steps;
} module_globals_t;

extern module_globals_t module_globals;

/* Module status */
typedef enum {
    PERFEXPERT_MODULE_LOADED = 0,
    PERFEXPERT_MODULE_INITIALIZED,
    PERFEXPERT_MODULE_FINALIZED,
} perfexpert_module_status_t;

/* Module order */
typedef enum {
    PERFEXPERT_MODULE_BEFORE = 0,
    PERFEXPERT_MODULE_AFTER,
    PERFEXPERT_MODULE_AVAILABLE,
    PERFEXPERT_MODULE_FIRST,
    PERFEXPERT_MODULE_LAST,
    PERFEXPERT_MODULE_CLONE_BEFORE,
    PERFEXPERT_MODULE_CLONE_AFTER,
} perfexpert_module_order_t;

/* Module interface */
typedef int (*perfexpert_module_load_fn_t)(void);
typedef int (*perfexpert_module_init_fn_t)(void);
typedef int (*perfexpert_module_fini_fn_t)(void);
typedef int (*perfexpert_module_compile_fn_t)(void);
typedef int (*perfexpert_module_instrument_fn_t)(void);
typedef int (*perfexpert_module_measure_fn_t)(void);
typedef int (*perfexpert_module_analyze_fn_t)(void);
typedef int (*perfexpert_module_recommend_fn_t)(void);

/* Base module structure. Any module should implement these functions */
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char *version;
    int  argc;
    char *argv[MAX_ARGUMENTS_COUNT];
    perfexpert_module_status_t status;
    perfexpert_module_load_fn_t load;
    perfexpert_module_init_fn_t init;
    perfexpert_module_fini_fn_t fini;
    perfexpert_module_compile_fn_t compile;
    perfexpert_module_instrument_fn_t instrument;
    perfexpert_module_measure_fn_t measure;
    perfexpert_module_analyze_fn_t analyze;
    perfexpert_module_recommend_fn_t recommend;
    /* Extended module interface from this point */
} perfexpert_module_1_0_0_t;

typedef perfexpert_module_1_0_0_t perfexpert_module_t;

/* Step status */
typedef enum {
    PERFEXPERT_STEP_UNDEFINED = -2,
    PERFEXPERT_STEP_FAILURE,
    PERFEXPERT_STEP_SUCCESS,
    PERFEXPERT_STEP_ERROR,
    PERFEXPERT_STEP_NOREC,
    PERFEXPERT_STEP_NOTRANS,
} perfexpert_step_status_t;

/* Step phase */
typedef enum {
    PERFEXPERT_PHASE_COMPILE = 0,
    PERFEXPERT_PHASE_INSTRUMENT,
    PERFEXPERT_PHASE_MEASURE,
    PERFEXPERT_PHASE_ANALYZE,
    PERFEXPERT_PHASE_RECOMMEND,
    PERFEXPERT_PHASE_UNDEFINED,
} perfexpert_step_phase_t;

/* Workflow step structure */
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_module_t *module;
    perfexpert_step_phase_t phase;
    perfexpert_step_status_t status;
    int (*function)();
    char *name;
} perfexpert_step_t;

/* Function declaration */
void __attribute__ ((constructor)) my_init(void);
int perfexpert_module_load(const char *name);
int perfexpert_module_set_option(const char *module, const char *option);
int perfexpert_module_init(void);
int perfexpert_module_fini(void);
int perfexpert_module_requires(const char *a, perfexpert_step_phase_t ap,
    const char *b, perfexpert_step_phase_t bp,
    perfexpert_module_order_t order);
perfexpert_module_t *perfexpert_module_get(const char *name);
int perfexpert_module_available(const char *name);
int perfexpert_phase_available(perfexpert_step_phase_t phase);
int perfexpert_module_installed(const char *name);

static lt_dlhandle perfexpert_module_open(const char *name);
static int perfexpert_module_close(lt_dlhandle handle, const char *name);
static int perfexpert_phase_add(perfexpert_module_t *m,
    perfexpert_step_phase_t p);
static perfexpert_step_t* perfexpert_step_clone(perfexpert_step_t *s);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_BASE_MODULE_H_ */
