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
#define PERFEXPERT_TOOL_HPCTOOLKIT_COUNTERS      "papi"
#define PERFEXPERT_TOOL_HPCTOOLKIT_TOT_INS       "PAPI_TOT_INS"
#define PERFEXPERT_TOOL_HPCTOOLKIT_TOT_CYC       "PAPI_TOT_CYC"

/* Function declarations */
int hpctoolkit_parse_file(const char *file, perfexpert_list_t *profiles);
static int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_HPCTOOLKIT_H_ */
