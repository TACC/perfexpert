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

#ifndef ANALYZER_VTUNE_H_
#define ANALYZER_VTUNE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "perfexpert_list.h"

/* VTune stuff */
#define PERFEXPERT_TOOL_VTUNE_COUNTERS      "vtune"
#define PERFEXPERT_TOOL_VTUNE_TOT_INS       "PAPI_TOT_INS"
#define PERFEXPERT_TOOL_VTUNE_TOT_CYC       "PAPI_TOT_CYC"

/* Function declarations */
int vtune_parse_file(const char *file, perfexpert_list_t *profiles);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_VTUNE_H_ */
