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
#include "macvec_module.h"
#include "macvec.h"
#include "macvec_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"

/* Global variable to define the module itself */
perfexpert_module_macvec_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Initialize list of events */
    perfexpert_list_construct(&(my_module_globals.profiles));
    my_module_globals.architecture = NULL;

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
    }

    /* Should we use HPCToolkit? */
    if (PERFEXPERT_TRUE == perfexpert_module_available("hpctoolkit")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use HPCToolkit as measurement module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
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
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
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
        char *error = NULL, sql[MAX_BUFFER_SIZE];
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

    /* Initialize the measurements module before using it */
    if (PERFEXPERT_MODULE_LOADED == my_module_globals.measurement->status) {
        if (PERFEXPERT_SUCCESS != my_module_globals.measurement->init()) {
            OUTPUT(("%s [%s]", _ERROR("error initializing module"),
                my_module_globals.measurement->name));
            return PERFEXPERT_ERROR;
        }
    }

    OUTPUT(("%s", _YELLOW("Setting performance events")));

    /* Jaketown (or SandyBridgeEP) */
    if (0 == strcmp("jaketown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != counters_jaketown()) {
            OUTPUT(("%s", _ERROR("setting counters (Jaketown)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* MIC (or KnightsCorner) */
    else if (0 == strcmp("mic",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != metrics_mic()) {
            OUTPUT(("%s", _ERROR("setting counters (MIC)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* Unknown */
    else if (0 == strcmp("unknown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != counters_papi()) {
            OUTPUT(("%s", _ERROR("setting counters (PAPI)")));
            return PERFEXPERT_ERROR;
        }
    }
    /* If not any of the above, I'm sorry... */
    else {
        OUTPUT(("%s (%s)", _ERROR("setting counters"),
            my_module_globals.architecture));
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

static int cmp_relevance(const macvec_hotspot_t **a,
        const macvec_hotspot_t **b) {
    if ((*a)->importance > (*b)->importance) {
        return -1;
    }

    if ((*a)->importance < (*b)->importance) {
        return +1;
    }

    return 0;
}

int filter_and_sort_hotspots(perfexpert_list_t* hotspots, double threshold) {
    if (0 == perfexpert_list_get_size(hotspots)) {
        return PERFEXPERT_SUCCESS;
    }

    perfexpert_list_item_t **items, *item;
    items = (perfexpert_list_item_t**)malloc(sizeof(perfexpert_list_item_t*) *
            perfexpert_list_get_size(hotspots));

    if (NULL == items) {
        return PERFEXPERT_ERROR;
    }

    int index = 0;
    while (NULL != (item = perfexpert_list_get_first(hotspots))) {
        perfexpert_list_remove_item(hotspots, item);
        macvec_hotspot_t* hotspot = (macvec_hotspot_t*) item;
        if (hotspot->importance >= threshold &&
                hotspot->type == PERFEXPERT_HOTSPOT_LOOP) {
            items[index++] = item;
        }
    }

    qsort(items, index, sizeof(perfexpert_list_item_t*),
            (int(*)(const void*, const void*)) cmp_relevance);

    int n;
    for (n = 0; n < index; n++) {
        perfexpert_list_append(hotspots, items[n]);
    }

    free(items);

    return PERFEXPERT_SUCCESS;
}
/* module_analyze */
int module_analyze(void) {
    macvec_profile_t *p = NULL;
    macvec_hotspot_t *h = NULL;

    OUTPUT(("%s", _YELLOW("Analyzing measurements")));

    /* Import measurements to memory */
    if (PERFEXPERT_SUCCESS != database_import(&(my_module_globals.profiles),
        my_module_globals.measurement->name)) {
        OUTPUT(("%s", _ERROR("unable to import profiles")));
        return PERFEXPERT_ERROR;
    }

    /* For each profile... */
    macvec_profile_t* profile;
    double threshold = my_module_globals.threshold;
    perfexpert_list_for(profile, &(my_module_globals.profiles),
            macvec_profile_t) {
        perfexpert_list_t* hotspots = &(profile->hotspots);
        filter_and_sort_hotspots(hotspots, threshold);
        process_hotspots(hotspots);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
