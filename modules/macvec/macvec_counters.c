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

/* System standard headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Utility headers */

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "macvec.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

/* counters_jaketown */
int counters_jaketown(void) {
    OUTPUT_VERBOSE((4, "%s", _YELLOW("setting counters (Intel native names)")));

    /* Set the profile total cycles and total instructions counters */
    strcpy(my_module_globals.measurement->total_cycles_counter, "CPU_CLK_UNHALTED.THREAD_P");
    strcpy(my_module_globals.measurement->total_inst_counter, "INST_RETIRED.ANY_P");

    /* Set the events on the measurement module */
    USE_EVENT("CPU_CLK_UNHALTED.THREAD_P");
    USE_EVENT("INST_RETIRED.ANY_P");

    return PERFEXPERT_SUCCESS;
}

/* counters_mic */
int counters_mic(void) {
    OUTPUT_VERBOSE((4, "%s", _YELLOW("setting counters (PAPI)")));

    /* Set the profile total cycles and total instructions counters */
    strcpy(my_module_globals.measurement->total_cycles_counter, "CPU_CLK_UNHALTED");
    strcpy(my_module_globals.measurement->total_inst_counter, "INSTRUCTIONS_EXECUTED");

    /* Set the events on the measurement module */
    USE_EVENT("CPU_CLK_UNHALTED");
    USE_EVENT("INSTRUCTIONS_EXECUTED");

    return PERFEXPERT_SUCCESS;
}

/* counters_papi */
int counters_papi(void) {
    OUTPUT_VERBOSE((4, "%s", _YELLOW("setting counters (PAPI)")));

    /* Set the profile total cycles and total instructions counters */
    strcpy(my_module_globals.measurement->total_cycles_counter, "PAPY_TOT_CYC");
    strcpy(my_module_globals.measurement->total_inst_counter, "PAPI_TOT_INS");

    /* Set the events on the measurement module */
    USE_EVENT("PAPI_TOT_CYC");
    USE_EVENT("PAPI_TOT_INS");

    return PERFEXPERT_SUCCESS;
}

// EOF
