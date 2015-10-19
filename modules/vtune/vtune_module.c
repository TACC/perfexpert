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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Module headers */
#include "vtune.h"
#include "vtune_database.h"
#include "vtune_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
// #include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"

/* Global variable to define the module itself */
perfexpert_module_vtune_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    /* Extended interface */
    myself_module.set_event = NULL;
    myself_module.query_event = NULL;

    /* TODO(agomez): check for VTune binaries availability here */

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Extended interface */

    myself_module.set_event   = &module_set_event;
    myself_module.query_event = &module_query_event;

    /* Initialize list of events */
    my_module_globals.events_by_name = NULL;
    perfexpert_list_construct(&(myself_module.profiles));
    my_module_globals.mic = NULL;
    my_module_globals.inputfile = NULL;
    my_module_globals.ignore_return_code = PERFEXPERT_FALSE;

    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing module arguments")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    vtune_event_t *event = NULL, *tmp = NULL;

    /* Extended interface */
    myself_module.set_event = NULL;
    myself_module.query_event = NULL;

    /* Free the list of events */
    /* TODO(agomez) make sure this is all correct */

    /*
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, tmp) {
        perfexpert_hash_del_str(my_module_globals.events_by_name, event);
        PERFEXPERT_DEALLOC(event->name);
        PERFEXPERT_DEALLOC(event);
    }

    vtune_hw_profile_t *it;
    perfexpert_list_for (it, &(myself_module.profiles), vtune_hw_profile_t) {
        PERFEXPERT_DEALLOC (it->name);
        vtune_hotspots_t *h;
        perfexpert_list_for (h, &(it->hotspots), vtune_hotspots_t) {
            PERFEXPERT_DEALLOC (h->name);
            PERFEXPERT_DEALLOC (h->module);
            PERFEXPERT_DEALLOC (h->src_file);
            vtune_event_t *e;
            perfexpert_list_for(e, &(h->events), vtune_event_t) {
                PERFEXPERT_DEALLOC (e->name);
            }
        }
    }

*/
    my_module_globals.events_by_name = NULL;
    my_module_globals.ignore_return_code = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    char *str = NULL;

    OUTPUT(("%s (%d events)", _YELLOW("Collecting measurements"),
        perfexpert_hash_count_str(my_module_globals.events_by_name)));

    if (PERFEXPERT_SUCCESS != database_default_events(/*my_module_globals*/)) {
        OUTPUT(("%s", _ERROR("reading default events from the database")));
    }

    if (0 == perfexpert_hash_count_str(my_module_globals.events_by_name)) {
        OUTPUT_VERBOSE((5, "adding events to collect"));
        if (PERFEXPERT_SUCCESS != module_set_event("CPU_CLK_UNHALTED.REF_TSC")) {
            OUTPUT(("%s", _ERROR("ADDING EVENTS [CPU_CLK_UNHALTED.REF_TSC]")));
        }
        if (PERFEXPERT_SUCCESS != module_set_event("CPU_CLK_UNHALTED.REF_TSC")) {
        }
    }

    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS !=
        perfexpert_util_file_is_exec(globals.program_full)) {
        return PERFEXPERT_ERROR;
    }

    /* Collect measurements */
    if (NULL == my_module_globals.mic) {
        if (PERFEXPERT_SUCCESS != run_amplxe_cl()) {
            OUTPUT(("%s", _ERROR("unable to run amplxe-cl")));
            return PERFEXPERT_ERROR;
        }
     } else {
         if (PERFEXPERT_SUCCESS != run_amplxe_cl_mic()) {
             OUTPUT(("%s", _ERROR("unable to run amplxecl on MIC")));
             OUTPUT(("Are you adding the flags to compile for MIC?"));
             return PERFEXPERT_ERROR;
         }
    }

    vtune_hw_profile_t *profile;

    PERFEXPERT_ALLOC (vtune_hw_profile_t, profile, sizeof(vtune_hw_profile_t));
    profile->hotspots_by_name = NULL;
    perfexpert_list_item_construct((perfexpert_list_item_t *)profile);

    perfexpert_list_construct(&(profile->hotspots));

    /* Add the individual profile to the list of profiles global to the 
       module */

    /* TODO(agomez): this line makes lcpi fail. I have to figure out why */
    /* perfexpert_list_append (&(myself_module.profiles), 
                               (perfexpert_list_item_t *) profile); 
    */

    if (PERFEXPERT_SUCCESS != collect_results(profile)) {
        OUTPUT(("%s",
                _ERROR("unable to collect the results generated by VTUNE")));
        PERFEXPERT_DEALLOC(profile);
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != database_hw_events(profile)) {
        PERFEXPERT_DEALLOC(profile);
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_DEALLOC(profile);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
