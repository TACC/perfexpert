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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <float.h>

/* Module headers */
#include "timb_module.h"
#include "timb.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

/* Global variable to define the module itself */
perfexpert_module_timb_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Module pre-requisites */
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("timb",
        PERFEXPERT_PHASE_MEASURE, "hpctoolkit", PERFEXPERT_PHASE_MEASURE,
        PERFEXPERT_MODULE_AFTER)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("timb",
        PERFEXPERT_PHASE_ANALYZE, "hpctoolkit", PERFEXPERT_PHASE_MEASURE,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }

    /* Initialize some variables */
    my_module_globals.maximum = DBL_MIN;
    my_module_globals.minimum = DBL_MAX;
    my_module_globals.threads = 0;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    OUTPUT(("%s", _YELLOW("Setting performance events")));

    my_module_globals.hpctoolkit = (perfexpert_module_hpctoolkit_t *)
        perfexpert_module_get("hpctoolkit");

    if (PERFEXPERT_SUCCESS != my_module_globals.hpctoolkit->set_event(
        "PAPI_TOT_CYC")) {
        OUTPUT(("%s", _ERROR("unable to generate add PAPI_TOT_CYC")));
        return PERFEXPERT_ERROR;
    }

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
