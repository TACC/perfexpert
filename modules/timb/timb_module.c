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

    /* Check if at least one of HPCToolkit or VTune is loaded */
    if ((PERFEXPERT_FALSE == perfexpert_module_available("hpctoolkit")) &&
        (PERFEXPERT_FALSE == perfexpert_module_available("vtune"))) {
        OUTPUT(("%s", _RED("Neither HPCToolkit nor VTune module loaded")));

        /* Default to HPCToolkit */
        OUTPUT(("%s", _RED("PerfExpert will try to load HPCToolkit module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_load("hpctoolkit")) {
            OUTPUT(("%s", _ERROR("while loading HPCToolkit module")));
            return PERFEXPERT_ERROR;
        }   

        /* Get the module address */
        if (NULL == (my_module_globals.hpctoolkit =
                (perfexpert_module_hpctoolkit_t *)
                perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("unable to get measurements module")));
            return PERFEXPERT_SUCCESS;
        }   
    }   

    if (PERFEXPERT_TRUE == perfexpert_module_available("hpctoolkit")) {
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
        my_module_globals.measure_mod = HPCTOOLKIT_MOD;
    }

    if (PERFEXPERT_TRUE == perfexpert_module_available("vtune")) {
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("timb",
            PERFEXPERT_PHASE_MEASURE, "vtune", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_AFTER)) {
            OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("timb",
            PERFEXPERT_PHASE_ANALYZE, "vtune", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_BEFORE)) {
            OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        my_module_globals.measure_mod = VTUNE_MOD;
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

    if (my_module_globals.measure_mod == HPCTOOLKIT_MOD) {
        my_module_globals.hpctoolkit = (perfexpert_module_hpctoolkit_t *)
            perfexpert_module_get("hpctoolkit");

        if (PERFEXPERT_SUCCESS != my_module_globals.hpctoolkit->set_event(
            "PAPI_TOT_CYC")) {
            OUTPUT(("%s", _ERROR("unable to generate add PAPI_TOT_CYC")));
            return PERFEXPERT_ERROR;
        }
    }
    if (my_module_globals.measure_mod == VTUNE_MOD) {
        my_module_globals.vtune = (perfexpert_module_vtune_t *)
            perfexpert_module_get("vtune");

        if (PERFEXPERT_SUCCESS != my_module_globals.vtune->set_event(
            "CPU_CLK_UNHALTED.REF_TSC")) {
            OUTPUT(("%s", _ERROR("unable to generate add CPU_CLK_UNHALTED.REF_TSC")));
            return PERFEXPERT_ERROR;
        }
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
