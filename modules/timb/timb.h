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

#ifndef PERFEXPERT_MODULE_TIMB_H_
#define PERFEXPERT_MODULE_TIMB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "modules/hpctoolkit/hpctoolkit_module.h"
#include "modules/vtune/vtune_module.h"

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_timb]"

typedef enum{
    HPCTOOLKIT_MOD = 1,
    VTUNE_MOD = 2,
} perfexpert_timb_modules;

/* Module types */
typedef struct {
    perfexpert_module_hpctoolkit_t *hpctoolkit;
    perfexpert_module_vtune_t *vtune;
    int measure_mod;
    double total;
    double maximum;
    double minimum;
    int threads;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);
int module_analyze(void);

/* Module functions */
int parse_module_args(int argc, char *argv[]);
int output_analysis(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_TIMB_H_ */
