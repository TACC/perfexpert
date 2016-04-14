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
#include <papi.h>
#include <string.h>

/* Module headers */
#include "hpctoolkit.h"
#include "hpctoolkit_module.h"
#include "hpctoolkit_tools.h"

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

    // TODO: check for HPCToolkit binaries availability here

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    myself_module.status = PERFEXPERT_MODULE_LOADED;

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Extended interface */
    myself_module.set_event = &module_set_event;
    myself_module.total_inst_counter = NULL;
    myself_module.total_cycles_counter = NULL;
    perfexpert_list_construct(&(myself_module.profiles));

    /* Initialize list of events */
    my_module_globals.events_by_name = NULL;
    my_module_globals.prefix[0] = NULL;
    my_module_globals.before[0] = NULL;
    my_module_globals.after[0] = NULL;
    my_module_globals.mic_prefix[0] = NULL;
    my_module_globals.mic_before[0] = NULL;
    my_module_globals.mic_after[0] = NULL;
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
    myself_module.status = PERFEXPERT_MODULE_INITIALIZED;

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    hpctoolkit_event_t *event = NULL, *tmp = NULL;

    /* Extended interface */
    myself_module.set_event = NULL;
    myself_module.total_inst_counter = NULL;
    myself_module.total_cycles_counter = NULL;

    /* Free the list of events */
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, tmp) {
        perfexpert_hash_del_str(my_module_globals.events_by_name, event);
        PERFEXPERT_DEALLOC(event->name);
        PERFEXPERT_DEALLOC(event);
    }
    my_module_globals.events_by_name = NULL;
    my_module_globals.ignore_return_code = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));
    myself_module.status = PERFEXPERT_MODULE_FINALIZED;

    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    char *str = NULL;

    OUTPUT(("%s (%d events)", _YELLOW("Collecting measurements"),
        perfexpert_hash_count_str(my_module_globals.events_by_name)));

    /* Check if there is any event to collect */
    if (0 == perfexpert_hash_count_str(my_module_globals.events_by_name)) {
        OUTPUT_VERBOSE((5, "adding PAPI_TOT_INS and PAPI_TOT_CYC at least"));

        if (PERFEXPERT_SUCCESS != module_set_event("PAPI_TOT_INS")) {
            OUTPUT(("%s", _ERROR("adding events [PAPI_TOT_INS]")));
        }
        if (PERFEXPERT_SUCCESS != module_set_event("PAPI_TOT_CYC")) {
            OUTPUT(("%s", _ERROR("adding events [PAPI_TOT_CYC]")));
        }
        myself_module.total_inst_counter = "PAPI_TOT_INS";
        myself_module.total_cycles_counter = "PAPI_TOT_CYC";
    }

    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS !=
        perfexpert_util_file_is_exec(globals.program_full)) {
        return PERFEXPERT_ERROR;
    }

    /* Create the program structure file */
    if (PERFEXPERT_SUCCESS != run_hpcstruct()) {
        OUTPUT(("%s", _ERROR("unable to run hpcstruct")));
        return PERFEXPERT_ERROR;
    }

    /* Collect measurements */
    if (NULL == my_module_globals.mic) {
        if (PERFEXPERT_SUCCESS != run_hpcrun()) {
            OUTPUT(("%s", _ERROR("unable to run hpcrun")));
            return PERFEXPERT_ERROR;
        }
    } else {
        if (PERFEXPERT_SUCCESS != run_hpcrun_mic()) {
            OUTPUT(("%s", _ERROR("unable to run hpcrun on MIC")));
            OUTPUT(("Are you adding the flags to compile for MIC?"));
            return PERFEXPERT_ERROR;
        }
    }

    /* Sumarize results */
    if (PERFEXPERT_SUCCESS != run_hpcprof(&str)) {
        OUTPUT(("%s", _ERROR("unable to run hpcprof")));
        return PERFEXPERT_ERROR;
    }

    /* Parse results */
    if (PERFEXPERT_SUCCESS != parse_file(str)) {
        OUTPUT(("%s [%s]", _ERROR("unable to parse file"), str));
        return PERFEXPERT_ERROR;
    }

    /* Check profiles */
    if (PERFEXPERT_SUCCESS != profile_check_all(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("checking profile")));
        return PERFEXPERT_ERROR;
    }

    /* Flatten profiles */
    if (PERFEXPERT_SUCCESS != profile_flatten_all(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("flatening profiles")));
        return PERFEXPERT_ERROR;
    }

    /* Write profiles to database */
    if (PERFEXPERT_SUCCESS != database_profiles(&(myself_module.profiles))) {
        OUTPUT(("%s", _ERROR("writing profiles to database")));
        return PERFEXPERT_ERROR;
    }

    /*  Set number of tasks and threads for this experiment */
    if (PERFEXPERT_SUCCESS != database_set_tasks_threads()) {
        OUTPUT(("%s", _ERROR("writing tasks and threads to database")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* module_set_event */
int module_set_event(const char *name) {
    hpctoolkit_event_t *event = NULL;
    char *str = NULL;

    /* Make a local copy because we will need to change the string... */
    PERFEXPERT_ALLOC(char, str, (strlen(name) + 1));
    strcpy(str, name);

    /* PAPI requires to convert the event name, replacing '.' by ':' */
    perfexpert_string_replace_char(str, '.', ':');

    /* Check if event is already set or if it is PAPI_TOT_INS */
    perfexpert_hash_find_str(my_module_globals.events_by_name,
        perfexpert_md5_string(str), event);

    if ((NULL != event) ||
        (0 == strcmp(name, "PAPI_TOT_INS")) ||
        (0 == strcmp(name, "INST_RETIRED.ANY")) ||
        (0 == strcmp(name, "INST_RETIRED.ANY_P"))) {
        OUTPUT_VERBOSE((10, "event %s already set", _RED((char *)str)));
        PERFEXPERT_DEALLOC(str);
        return PERFEXPERT_SUCCESS;
    }

    /* Check if event exists */
    if (PERFEXPERT_SUCCESS != papi_check_event(str)) {
        OUTPUT(("%s [%s]", _ERROR("invalid event name"), name));
        return PERFEXPERT_ERROR;
    }

    /* Add event to the hash of events */
    PERFEXPERT_ALLOC(hpctoolkit_event_t, event, sizeof(hpctoolkit_event_t));
    PERFEXPERT_ALLOC(char, event->name, (strlen(str) + 1));
    strcpy(event->name, str);
    strcpy(event->name_md5, perfexpert_md5_string(str));
    perfexpert_hash_add_str(my_module_globals.events_by_name, name_md5, event);

    OUTPUT_VERBOSE((6, "event %s set", _CYAN(event->name)));

    PERFEXPERT_DEALLOC(str);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
