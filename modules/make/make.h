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

#ifndef PERFEXPERT_MODULE_MAKE_H_
#define PERFEXPERT_MODULE_MAKE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_make]"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Private module types */
// typedef struct {
//     char *target;
//     char *env[MAX_ARGUMENTS_COUNT];
// } my_module_globals_t;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_compile(void);

/* Extended Module interface */
int module_set_target(const char *target);
int module_set_env(const char *name, const char *value);
int module_unset_env(const char *name);
char* module_get_env(const char *name);

/* Module functions */
int run_make(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_MAKE_H_ */
