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

#ifndef PERFEXPERT_MODULE_H_
#define PERFEXPERT_MODULE_H_

#define MAX_MODULE_NAME 32

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include "perfexpert_list.h"

/* Module interface */
typedef int (*perfexpert_module_measurements_fn_t)(void);
typedef int (*perfexpert_module_parse_file_fn_t)
    (const char *file, perfexpert_list_t *profiles);

/* PerfExpert Module Structure. Any module should implement these functions */
struct perfexpert_module_1_0_0_t;
typedef struct perfexpert_module_1_0_0_t perfexpert_module_t;
typedef struct perfexpert_module_1_0_0_t {
    perfexpert_module_measurements_fn_t measurements;
    perfexpert_module_parse_file_fn_t   parse_file;
    char *profile_file;
    char *total_instructions;
    char *total_cycles;
    char *counter_prefix;
} perfexpert_module_1_0_0_t;

/* Function declaration */
int perfexpert_load_module(const char *toolname, perfexpert_module_t *module);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_H */
