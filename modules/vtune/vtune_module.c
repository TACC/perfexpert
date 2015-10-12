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
//#include "common/perfexpert_md5.h"
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

    // TODO: check for VTune binaries availability here

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
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, tmp) {
        perfexpert_hash_del_str(my_module_globals.events_by_name, event);
        PERFEXPERT_DEALLOC(event->name);
        PERFEXPERT_DEALLOC(event);
    }
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
    //profile->events_list=NULL;
    profile->events_by_name = NULL;
    perfexpert_list_item_construct((perfexpert_list_item_t *)profile);
    perfexpert_list_construct(&(profile->events_list));
    OUTPUT_VERBOSE ((8, "%s %d", "INITIAL size of events_list", perfexpert_list_get_size(&(profile->events_list))));
    if (PERFEXPERT_SUCCESS != collect_results(profile)) {
        OUTPUT (("%s", _ERROR ("unable to collect the results generated by VTUNE")));
        PERFEXPERT_DEALLOC (profile);
        return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE((8, "%s", "results collected"));

    if (PERFEXPERT_SUCCESS != database_hw_events (profile)) {
        OUTPUT (("%s", _ERROR ("writing profiles to database")));
        PERFEXPERT_DEALLOC (profile);
        return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE((8,"%s", "results stored in the database"));

    PERFEXPERT_DEALLOC (profile);

    //if (PERFEXPERT_SUCCESS != collect_results()
    // /* Sumarize results */
    // if (PERFEXPERT_SUCCESS != run_hpcprof(&str)) {
    //     OUTPUT(("%s", _ERROR("unable to run hpcprof")));
    //     return PERFEXPERT_ERROR;
    // }

    // /* Parse results */
    // if (PERFEXPERT_SUCCESS != parse_file(str)) {
    //     OUTPUT(("%s [%s]", _ERROR("unable to parse file"), str));
    //     return PERFEXPERT_ERROR;
    // }

    // /* Check profiles */
    // if (PERFEXPERT_SUCCESS != profile_check_all(&(myself_module.profiles))) {
    //     OUTPUT(("%s", _ERROR("checking profile")));
    //     return PERFEXPERT_ERROR;
    // }

    // /* Flatten profiles */
    // if (PERFEXPERT_SUCCESS != profile_flatten_all(&(myself_module.profiles))) {
    //     OUTPUT(("%s", _ERROR("flatening profiles")));
    //     return PERFEXPERT_ERROR;
    // }

    // /* Write profiles to database */
    // if (PERFEXPERT_SUCCESS != database_profiles(&(myself_module.profiles))) {
    //     OUTPUT(("%s", _ERROR("writing profiles to database")));
    //     return PERFEXPERT_ERROR;
    // }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
