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

#ifndef PERFEXPERT_MODULE_SQLRULES_H_
#define PERFEXPERT_MODULE_SQLRULES_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_sqlrules]"

/* Structure to hold global variables */
typedef struct {
    int  no_rec;
    int  rec_count;
	FILE *metrics_FP;
    perfexpert_list_t rules;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* Function declarations */
int parse_cli_params(int argc, char *argv[]);
int select_recommendations(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_SQLRULES_H_ */
