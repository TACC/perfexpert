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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */

/* Module headers */
#include "macpo_module.h"
#include "macpo.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

/* Global variable to define the module itself */
perfexpert_module_macpo_t myself_module;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Module pre-requisites */
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macpo",
        PERFEXPERT_PHASE_INSTRUMENT, "lcpi", PERFEXPERT_PHASE_ANALYZE,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macpo",
        PERFEXPERT_PHASE_INSTRUMENT, NULL, PERFEXPERT_PHASE_COMPILE,
        PERFEXPERT_MODULE_CLONE_AFTER)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macpo",
        PERFEXPERT_PHASE_MEASURE, "macpo", PERFEXPERT_PHASE_INSTRUMENT,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macpo",
        PERFEXPERT_PHASE_ANALYZE, "macpo", PERFEXPERT_PHASE_MEASURE,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));
    return PERFEXPERT_SUCCESS;
}

/* module_instrument */
int module_instrument(void) {
    OUTPUT(("%s", _YELLOW("Instrumenting the code")));

    if (PERFEXPERT_SUCCESS != macpo_instrument_all()) {
        OUTPUT(("%s", _ERROR("instrumenting files")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    OUTPUT(("%s", _YELLOW("Collecting measurements")));

    return PERFEXPERT_SUCCESS;
}

/* module_analyze */
int module_analyze(void) {
    OUTPUT(("%s", _YELLOW("Analysing measurements")));

    if (PERFEXPERT_SUCCESS != macpo_analyze()) {
        OUTPUT(("%s", _ERROR("analyzing MACPO result")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
