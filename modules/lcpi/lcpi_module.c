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

/* Module headers */
#include "modules/perfexpert_module_base.h"
#include "modules/perfexpert_module_measurement.h"
#include "lcpi_module.h"
#include "lcpi.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"

/* Global variable to define the module itself */
perfexpert_module_lcpi_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    lcpi_metric_t *m = NULL, *t = NULL;

    /* Initialize list of events */
    perfexpert_list_construct(&(my_module_globals.profiles));
    my_module_globals.metrics_by_name = NULL;
    my_module_globals.threshold = 0.0;
    my_module_globals.help_only = PERFEXPERT_FALSE;
    my_module_globals.measurement = NULL;
    my_module_globals.architecture = NULL;
    my_module_globals.mic = PERFEXPERT_FALSE;
    my_module_globals.verbose = 0;

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
        if (NULL == (my_module_globals.measurement =
                (perfexpert_module_measurement_t *)
                perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("unable to get measurements module")));
            return PERFEXPERT_SUCCESS;
        }

        /* Initialize the measurements module before using it */
        if (PERFEXPERT_MODULE_LOADED == my_module_globals.measurement->status) {
            if (PERFEXPERT_SUCCESS != my_module_globals.measurement->init()) {
                OUTPUT(("%s [%s]", _ERROR("error initializing module"),
                    my_module_globals.measurement->name));
                return PERFEXPERT_ERROR;
            }
        }
        my_module_globals.measurement->status = PERFEXPERT_MODULE_INITIALIZED;
    }

    /* Should we use HPCToolkit? */
    if (PERFEXPERT_TRUE == perfexpert_module_available("hpctoolkit")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use HPCToolkit as measurement module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("lcpi",
            PERFEXPERT_PHASE_ANALYZE, "hpctoolkit", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_BEFORE)) {
            OUTPUT(("%s", _ERROR("required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        if (NULL == (my_module_globals.measurement =
            (perfexpert_module_measurement_t *)
            perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("required module not available")));
            return PERFEXPERT_ERROR;
        }
    }

    /* Should we generate metrics to use with Vtune? */
    if (PERFEXPERT_TRUE == perfexpert_module_available("vtune")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use VTune as measurement module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("lcpi",
            PERFEXPERT_PHASE_ANALYZE, "vtune", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_BEFORE)) {
            OUTPUT(("%s", _ERROR("required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        if (NULL == (my_module_globals.measurement =
            (perfexpert_module_measurement_t *)
            perfexpert_module_get("vtune"))) {
            OUTPUT(("%s", _ERROR("required module not available")));
            return PERFEXPERT_ERROR;
        }
    }

    /* Triple check: at least one measurement module should be available */
    if (NULL == my_module_globals.measurement) {
        OUTPUT(("%s", _ERROR("No measurement module loaded")));
        return PERFEXPERT_ERROR;
    }

    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing module arguments")));
        return PERFEXPERT_ERROR;
    }

    /* If the architecture was not set, we should try to identify it... */
    if (NULL == my_module_globals.architecture) {
        int family = perfexpert_cpuinfo_get_family();
        int model  = perfexpert_cpuinfo_get_model();

        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT description FROM arch_processor WHERE family=%d "
            "AND model=%d;", family, model);
        OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            perfexpert_database_get_string,
            (void *)&my_module_globals.architecture, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        if (NULL == my_module_globals.architecture) {
            OUTPUT_VERBOSE((1, "Unknown architecture, using PAPI defaults"));
            my_module_globals.architecture = "unknown";
        }

        OUTPUT_VERBOSE((1, "Architecture not set but it looks like [%s]",
            my_module_globals.architecture));
    }

    OUTPUT(("%s", _YELLOW("Setting performance events")));
    /* Jaketown (or SandyBridgeEP) */
    if (0 == strcmp("jaketown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != metrics_jaketown()) {
            OUTPUT(("%s", _ERROR("generating LCPI metrics (Jaketown)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* MIC (or KnightsCorner) */
    else if (0 == strcmp("mic",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != metrics_mic()) {
            OUTPUT(("%s", _ERROR("generating LCPI metrics (MIC)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* Unknown */
    else if (0 == strcmp("unknown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != metrics_papi()) {
            OUTPUT(("%s", _ERROR("generating LCPI metrics (PAPI)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* If not any of the above, I'm sorry... */
    else {
        OUTPUT(("%s (%s)", _ERROR("generating LCPI metrics"),
            my_module_globals.architecture));
        return PERFEXPERT_ERROR;
    }

    /* Show the list of metrics */
    perfexpert_hash_iter_str(my_module_globals.metrics_by_name, m, t) {
        OUTPUT_VERBOSE((7, "   %s=%s", _CYAN(m->name),
            evaluator_get_string(m->expression)));
    }

    OUTPUT_VERBOSE((4, "(%d) %s",
        perfexpert_hash_count_str(my_module_globals.metrics_by_name),
        _MAGENTA("LCPI metrics")));
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_analyze */
int module_analyze(void) {
    lcpi_profile_t *p = NULL;

    OUTPUT(("%s", _YELLOW("Analyzing measurements")));

    // TODO: Wrap all these functions in a SQL transaction

    if (PERFEXPERT_SUCCESS != database_import(&(my_module_globals.profiles),
        my_module_globals.measurement->name)) {
        OUTPUT(("%s", _ERROR("unable to import profiles")));
        return PERFEXPERT_ERROR;
    }

    perfexpert_list_for(p, &(my_module_globals.profiles), lcpi_profile_t) {
        if (PERFEXPERT_SUCCESS != logic_lcpi_compute(p)) {
            OUTPUT(("%s", _ERROR("unable to compute LCPI")));
            return PERFEXPERT_ERROR;
        }
    }

    if (NULL != my_module_globals.order) {
        if (PERFEXPERT_SUCCESS != hotspot_sort(&(my_module_globals.profiles))) {
            OUTPUT(("%s", _ERROR("sorting hotspots")));
            return PERFEXPERT_ERROR;
        }
    }

    if (PERFEXPERT_SUCCESS != output_analysis(&(my_module_globals.profiles))) {
        OUTPUT(("%s", _ERROR("printing analysis report")));
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != database_export(&(my_module_globals.profiles))) {
        OUTPUT(("%s", _ERROR("writing metrics to database")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
