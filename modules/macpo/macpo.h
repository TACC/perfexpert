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
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_MACPO_H_
#define PERFEXPERT_MODULE_MACPO_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "macpo_types.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_macpo]"

typedef struct {
    char *prefix[MAX_ARGUMENTS_COUNT];
    char *before[MAX_ARGUMENTS_COUNT];
    char *after[MAX_ARGUMENTS_COUNT];
    //char *inputfile;
    char res_folder[MAX_FILENAME];
    int ignore_return_code;
} my_module_globals_t;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_instrument(void);
int module_measure(void);
int module_analyze(void);

/* Module functions */
int macpo_instrument_all(void);
static int macpo_instrument(void *n, int c, char **val, char **names);
int macpo_compile(void);
int macpo_run(void);
int macpo_analyze(void);

extern my_module_globals_t my_module_globals;

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_MACPO_H_ */
