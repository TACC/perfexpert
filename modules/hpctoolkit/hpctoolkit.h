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

#ifndef ANALYZER_HPCTOOLKIT_H_
#define ANALYZER_HPCTOOLKIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XML_PARSER_H__
#include <libxml/parser.h>
#endif

#include "perfexpert_list.h"

/* HPCToolkit stuff */
#define PERFEXPERT_TOOL_EXPERIMENT_FILE     "experiment_hpctoolkit.conf"
#define PERFEXPERT_TOOL_MIC_EXPERIMENT_FILE "experiment_hpctoolkit_mic.conf"
#define PERFEXPERT_TOOL_PROFILE_FILE        "database/experiment.xml"
#define PERFEXPERT_TOOL_COUNTER_PREFIX      "papi"
#define PERFEXPERT_TOOL_TOTAL_INSTRUCTION   "PAPI_TOT_INS"
#define PERFEXPERT_TOOL_TOTAL_CYCLES        "PAPI_TOT_CYC"

/* HPCToolkit stuff (binaries should be in the path) */
#define HPCSTRUCT "hpcstruct"
#define HPCRUN    "hpcrun"
#define HPCPROF   "hpcprof"

/* Module interface */
int perfexpert_tool_measurements(void);
int perfexpert_tool_parse_file(const char *file, perfexpert_list_t *profiles);
char perfexpert_tool_profile_file[] = PERFEXPERT_TOOL_PROFILE_FILE;
char perfexpert_tool_total_instructions[] = PERFEXPERT_TOOL_TOTAL_INSTRUCTION;
char perfexpert_tool_total_cycles[] = PERFEXPERT_TOOL_TOTAL_CYCLES;
char perfexpert_tool_counter_prefix[] = PERFEXPERT_TOOL_COUNTER_PREFIX;

/* Module function declarations */
static int run_hpcstruct(void);
static int run_hpcrun(void);
static int run_hpcrun_knc(void);
static int run_hpcprof(void);
static int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_HPCTOOLKIT_H_ */
