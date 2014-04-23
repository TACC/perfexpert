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

#ifndef PERFEXPERT_MODULE_LCPI_METRICS_MIC_PAPI_H_
#define PERFEXPERT_MODULE_LCPI_METRICS_MIC_PAPI_H_

#define MAX_LCPI 1024

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"

#define USE_EVENT(X)                                                        \
    if (PERFEXPERT_SUCCESS != my_module_globals.hpctoolkit->set_event(X)) { \
        return PERFEXPERT_ERROR;                                            \
    }

/* Function declarations */
static int generate_mic_ratio_floating_point(void);
static int generate_mic_ratio_data_accesses(void);
static int generate_mic_overall(void);
static int generate_mic_data_accesses(void);
static int generate_mic_instruction_accesses(void);
static int generate_mic_tlb_metrics(void);
static int generate_mic_branch_metrics(void);
static int generate_mic_floating_point_instr(void);

#endif /* PERFEXPERT_MODULE_LCPI_METRICS_MIC_PAPI_H_ */
