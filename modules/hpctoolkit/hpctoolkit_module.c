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
#include <papi.h>
#include <string.h>

/* Module headers */
#include "hpctoolkit.h"
#include "hpctoolkit_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"

/* Global variable to define the module itself */
perfexpert_module_hpctoolkit_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {

    /* Extended interface */
    myself_module.set_event = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {

    /* Extended interface */
    myself_module.set_event = &module_set_event;

    /* Initialize list of events */
    my_module_globals.events_by_name = NULL;
    perfexpert_list_construct(&(myself_module.profiles));
    my_module_globals.knc = NULL;
    my_module_globals.inputfile = NULL;

    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing module arguments")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    hpctoolkit_event_t *event = NULL, *tmp = NULL;

    /* Extended interface */
    myself_module.set_event = NULL;

    /* Free the list of events */
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, tmp) {
        perfexpert_hash_del_str(my_module_globals.events_by_name, event);
        PERFEXPERT_DEALLOC(event->name);
        PERFEXPERT_DEALLOC(event);
    }
    my_module_globals.events_by_name = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_measurements */
int module_measurements(void) {
    char *workdir = NULL, *file = NULL;

    OUTPUT(("%s", _YELLOW("Collecting measurements")));

    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS !=
        perfexpert_util_file_is_exec(globals.program_full)) {
        return PERFEXPERT_ERROR;
    }

    /* Create working directory */
    PERFEXPERT_ALLOC(char, workdir, (strlen(globals.stepdir) + 12));
    sprintf(workdir, "%s/hpctoolkit", globals.stepdir);
    perfexpert_util_make_path(workdir);
    PERFEXPERT_DEALLOC(workdir);

    /* Create the program structure file */
    if (PERFEXPERT_SUCCESS != run_hpcstruct()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcstruct")));
        return PERFEXPERT_ERROR;
    }

    /* Collect measurements */
    if (NULL == my_module_globals.knc) {
        if (PERFEXPERT_SUCCESS != run_hpcrun()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun")));
            return PERFEXPERT_ERROR;
        }
    } else {
        if (PERFEXPERT_SUCCESS != run_hpcrun_knc()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun on KNC")));
            OUTPUT(("Are you adding the flags to compile for MIC?"));
            return PERFEXPERT_ERROR;
        }
    }

    /* Sumarize results */
    if (PERFEXPERT_SUCCESS != run_hpcprof(&file)) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcprof")));
        return PERFEXPERT_ERROR;
    }

    /* Parse results */
    if (PERFEXPERT_SUCCESS != parse_file(file)) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to parse file"), file));
        return PERFEXPERT_ERROR;
    }

    /* Check profiles */
    if (PERFEXPERT_SUCCESS != profile_check_all(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("Error: checking profile")));
        return PERFEXPERT_ERROR;
    }

    /* Flatten profiles */
    if (PERFEXPERT_SUCCESS != profile_flatten_all(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("Error: flatening profiles")));
        return PERFEXPERT_ERROR;
    }

    /* Write profiles to database */
    if (PERFEXPERT_SUCCESS != database_profiles(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("Error: writing profiles to database")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* set_event */
int module_set_event(const char *name) {
    hpctoolkit_event_t *event = NULL;
    int event_code;

    /* Check if event is already set or if it is PAPI_TOT_INS */
    perfexpert_hash_find_str(my_module_globals.events_by_name,
        perfexpert_md5_string(name), event);

    if ((NULL != event) || (0 == strcmp(name, "PAPI_TOT_INS"))) {
        OUTPUT_VERBOSE((10, "event %s already set", _RED((char *)name)));
        return PERFEXPERT_SUCCESS;
    }

    /* Check if event exists */
    if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
        OUTPUT(("%s", _ERROR("Error: while initializing PAPI")));
        return PERFEXPERT_ERROR;
    }
    if (PAPI_OK != PAPI_event_name_to_code((char *)name, &event_code)) {
        OUTPUT(("%s [%s]", _ERROR("Error: event not available (name->code)"),
            name));
        return HPCTOOLKIT_INVALID_EVENT;
    }
    if (PAPI_OK != PAPI_query_event(event_code)) {
        OUTPUT(("%s [%s]", _ERROR("Error: event not available (query code)"),
            name));
        return HPCTOOLKIT_INVALID_EVENT;
    }

    /* Add event to the hash of events */
    PERFEXPERT_ALLOC(hpctoolkit_event_t, event, sizeof(hpctoolkit_event_t));
    PERFEXPERT_ALLOC(char, event->name, (strlen(name) + 1));
    strcpy(event->name, name);
    strcpy(event->name_md5, perfexpert_md5_string(name));
    perfexpert_hash_add_str(my_module_globals.events_by_name, name_md5, event);

    OUTPUT_VERBOSE((6, "event %s set", _CYAN(event->name)));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
