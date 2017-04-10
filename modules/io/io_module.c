/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <float.h>

/* Module headers */
#include "io_module.h"
#include "io.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"

/* Global variable to define the module itself */
perfexpert_module_io_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    int i;
    /* Module pre-requisites */

    /* Initialize some variables */
    my_module_globals.maximum = DBL_MIN;
    my_module_globals.minimum = DBL_MAX;
    my_module_globals.threads = 0;
    for (i=0; i<MAX_FUNCTIONS; ++i) {
        my_module_globals.data[i].size=0;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    int i;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    for (i =0; i < MAX_FUNCTIONS; ++i) {
        PERFEXPERT_DEALLOC(my_module_globals.data[i].code);
    }
    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    return PERFEXPERT_SUCCESS;
}

/* module_analyze */
int module_analyze(void) {
    OUTPUT(("%s", _YELLOW("Analysing measurements")));

    if (PERFEXPERT_SUCCESS != output_analysis()) {
        OUTPUT(("%s", _ERROR("printing analysis report")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
