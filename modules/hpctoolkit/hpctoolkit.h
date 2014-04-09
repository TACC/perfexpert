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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_hpctoolkit]"

#ifndef __XML_PARSER_H__
#include <libxml/parser.h>
#endif

/* Modules headers */
#include "hpctoolkit_module.h"
#include "hpctoolkit_types.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* Private module types */
typedef struct {
    char *prefix[MAX_ARGUMENTS_COUNT];
    char *before[MAX_ARGUMENTS_COUNT];
    char *after[MAX_ARGUMENTS_COUNT];
    char *mic;
    char *mic_prefix[MAX_ARGUMENTS_COUNT];
    char *mic_before[MAX_ARGUMENTS_COUNT];
    char *mic_after[MAX_ARGUMENTS_COUNT];
    char *inputfile;
    hpctoolkit_event_t *events_by_name;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;
extern perfexpert_module_hpctoolkit_t myself_module;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);

/* Extended module interface */
int module_set_event(const char *name);

/* Function declarations */
int parse_file(const char *file);
int run_hpcstruct(void);
int run_hpcrun(void);
int run_hpcprof(char **file);
int mic_run_hpcrun(void);
int mic_run_hpcprof(char **file);
int profile_check_all(perfexpert_list_t *profiles);
int profile_flatten_all(perfexpert_list_t *profiles);
int database_profiles(perfexpert_list_t *profiles);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_H_ */
