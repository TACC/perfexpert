/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
#include "modules/perfexpert_module_base.h"
#include "modules/perfexpert_module_measurement.h"
#include "macvec_module.h"
#include "macvec.h"
#include "macvec_types.h"
#include "macvec_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"

/* Global variable to define the module itself */
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    int comp_loaded = PERFEXPERT_FALSE;

    my_module_globals.threshold = globals.threshold;
    /* Initialize list of events */
    perfexpert_list_construct(&(my_module_globals.profiles));

    // Check if at least one of HPCToolkit or VTune is loaded 
    if ((PERFEXPERT_FALSE == perfexpert_module_available("hpctoolkit")) &&
        (PERFEXPERT_FALSE == perfexpert_module_available("vtune"))) {
        OUTPUT(("%s", _RED("Neither HPCToolkit nor VTune module loaded")));

        // Default to HPCToolkit
        OUTPUT(("%s", _RED("PerfExpert will try to load HPCToolkit module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_load("hpctoolkit")) {
            OUTPUT(("%s", _ERROR("while loading HPCToolkit module")));
            return PERFEXPERT_ERROR;
        }

        // Get the module address
        if (NULL == (my_module_globals.measurement =
                (perfexpert_module_measurement_t *)
                perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("unable to get measurements module")));
            return PERFEXPERT_SUCCESS;
        }
    }

    // Should we use HPCToolkit?
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

    // Should we generate metrics to use with Vtune? 
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

    // Triple check: at least one measurement module should be available
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

    /*  Try to load a compilation module */
    if (PERFEXPERT_TRUE == perfexpert_module_available("make")) {
        myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("make");
        if (NULL != myself_module.measurement) {
            comp_loaded = PERFEXPERT_TRUE;
        }        
    }
    if (PERFEXPERT_TRUE == perfexpert_module_available("icc")) {
        myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("icc");
        if (NULL != myself_module.measurement) {
            comp_loaded = PERFEXPERT_TRUE;
        }        
    }
    if (PERFEXPERT_TRUE == perfexpert_module_available("gcc")) {
        OUTPUT((_ERROR("MACVEC only supports Intel compilers")));
        return PERFEXPERT_ERROR;
    }

    /*  If no compilation module is specified, finish */
    if (!comp_loaded) {
        OUTPUT(("%s", _ERROR(" this module needs a compilation module to be defined")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "finalizing"));
    /*  Deallocate the memory that was previously allocated */
    macvec_profile_t* profile;
    //if (my_module_globals.profiles) {
        perfexpert_list_for(profile, &(my_module_globals.profiles),
                macvec_profile_t) {
            PERFEXPERT_DEALLOC(profile->name);
            macvec_module_t *module, *t;
            perfexpert_hash_iter_str(profile->modules_by_name, module, t) {
                PERFEXPERT_DEALLOC(module->name);
                PERFEXPERT_DEALLOC(module);
            }
            macvec_hotspot_t *hotspot;
            perfexpert_list_for(hotspot, &(profile->hotspots), macvec_hotspot_t) {
                PERFEXPERT_DEALLOC(hotspot->name);
                PERFEXPERT_DEALLOC(hotspot->file);
                //      PERFEXPERT_DEALLOC(hotspot);
            }
            perfexpert_list_destruct(&(profile->hotspots));
            // This fails. I'm forgetting something else. TODO
            //   PERFEXPERT_DEALLOC(profile);
        }
        perfexpert_list_destruct (&(my_module_globals.profiles));
    // }
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

int filter_and_sort_hotspots(perfexpert_list_t* hotspots) {
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
        if (hotspot->importance >= my_module_globals.threshold &&
                hotspot->type == PERFEXPERT_HOTSPOT_LOOP) {
            items[index++] = item;
        }
    }

    qsort(items, index, sizeof(perfexpert_list_item_t*),
            (int(*)(const void*, const void*)) cmp_relevance);

    int n;
    for (n = 0; n < index; n++) {
        // Log hotspots.
        macvec_hotspot_t* hotspot = (macvec_hotspot_t*) items[n];
        OUTPUT_VERBOSE((4, "Found hotspot %s at %s:%ld [imp: %.2lf]",
                hotspot->name, hotspot->file, hotspot->line,
                hotspot->importance));
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
   
    /*  Append "-vec-report=6" to CFLAGS, CXXFLAGS and FCFLAGS */ 
    char *cflags = getenv("CFLAGS");
    char *newenv;
    if (cflags) {
        PERFEXPERT_ALLOC(char, newenv, strlen(cflags) + 15);
        snprintf(newenv, strlen(cflags) + 15,
                 "-vec-report=6 %s", cflags);
        if (setenv("CFLAGS", newenv, 1) < 0){
            return PERFEXPERT_ERROR;
        }
        PERFEXPERT_DEALLOC(newenv);
    }
    else {
        setenv ("CFLAGS", "-vec-report=6", 1);
    }

    char *cxxflags = getenv("CXXFLAGS");
    if (cxxflags) {
        PERFEXPERT_ALLOC(char, newenv, strlen(cxxflags) + 15);
        snprintf(newenv, strlen(cxxflags) + 15,
                 "-vec-report=6 %s", cxxflags); 
        if (setenv("CXXFLAGS", newenv, 1) < 0){
            return PERFEXPERT_ERROR;
        }
        PERFEXPERT_DEALLOC(newenv);
    }
    else {
        setenv("CXXFLAGS", "-vec-report=6", 1);
    } 
     
    char *fcflags = getenv("FCFLAGS");
    if (fcflags) {
        PERFEXPERT_ALLOC(char, newenv, strlen(fcflags) + 15);
        snprintf(newenv, strlen(fcflags) + 15,
                 "-vec-report=6 %s", fcflags); 
        if (setenv("FCFLAGS", newenv, 1) < 0){
            return PERFEXPERT_ERROR;
        }
        PERFEXPERT_DEALLOC(newenv);
    }
    else {
        setenv("FCFLAGS", "-vec-report=6", 1);
    }
    /*  Recompile the code with the new flags */    
    myself_module.measurement->compile();
 
  
    /*  Get the list of files with hotspots */
    perfexpert_list_t files;
    perfexpert_list_construct(&files);
    list_files_hotspots(&files);
    
    /*  Analyze the files with hotspots */
    char_t *filename;
    perfexpert_list_for (filename, &files, char_t) {
        macvec_profile_t* profile;
        database_import(&(my_module_globals.profiles), filename->name);
        perfexpert_list_for(profile, &(my_module_globals.profiles),
                macvec_profile_t) {
            perfexpert_list_t* hotspots = &(profile->hotspots);
            filter_and_sort_hotspots(hotspots);
            process_hotspots(hotspots, filename->name);
        }   
        PERFEXPERT_DEALLOC(filename->name);
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
