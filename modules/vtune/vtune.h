/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_MODULE_VTUNE_H_
#define PERFEXPERT_MODULE_VTUNE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_vtune]"

/* Modules headers */
#include "vtune_module.h"
#include "vtune_types.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"

extern my_module_globals_t my_module_globals;
extern perfexpert_module_vtune_t myself_module;

/* Module constants */
#define VTUNE_DATABASE ""
#define VTUNE_CL_BIN   "amplxe-cl"
#define VTUNE_ACT_COLLECT  "-collect"
#define VTUNE_ACT_COLLECTWITH "-collect-with"
#define VTUNE_ACT_REPORT   "-report"
#define VTUNE_RPT_HW       "hw-events"
/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);

/* Extended module interface */
int module_set_event(const char *name);
int module_query_event(const char *name);

/* Function declarations */
int create_report(char* results_folder, const char* parse_file);
int parse_line(char* line, char *argv[], int *argc);
int parse_report(const char * parse_file, vtune_hw_profile_t *profile);
int run_amplxe_cl(void);
int run_amplxe_cl_mic(void);
int collect_results(vtune_hw_profile_t *profile);
int get_thread_number(const char *);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_VTUNE_H_ */
