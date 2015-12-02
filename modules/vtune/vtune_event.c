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
#include "vtune_event.h"
#include "vtune_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
//#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

/* module_set_event */
int module_set_event(const char *name) {
    vtune_event_t *event = NULL;
    int event_code;

    /* Check if event is already set */
    perfexpert_hash_find_str(my_module_globals.events_by_name,
        perfexpert_md5_string(name), event);

    if (NULL != event) {
        OUTPUT_VERBOSE((10, "event %s already set", _RED((char *)name)));
        return PERFEXPERT_SUCCESS;
    }

    /* Check if this event is available in this architeture */
    if (NULL == my_module_globals.mic) {
        if (PERFEXPERT_TRUE != module_query_event(name)) {
            OUTPUT(("%s", _ERROR("VTune event not available")));
            return PERFEXPERT_ERROR;
        }
    }
    else {
        OUTPUT_VERBOSE((3, "event not checked because the architecture is MIC "
            "[%s]", name));
    }
    /* Add event to the hash of events */
    PERFEXPERT_ALLOC(vtune_event_t, event, sizeof(vtune_event_t));
    PERFEXPERT_ALLOC(char, event->name, (strlen(name) + 1));
    strcpy(event->name, name);
    strcpy(event->name_md5, perfexpert_md5_string(name));
    perfexpert_hash_add_str(my_module_globals.events_by_name, name_md5, event);

    OUTPUT_VERBOSE((6, "event %s set", _CYAN(event->name)));

    return PERFEXPERT_SUCCESS;
}

/* module_query_event */
int module_query_event(const char *name) {
    int i = 0, j = 0;

    OUTPUT_VERBOSE((9, "CPU Model %d - Checking for event %s", perfexpert_cpuinfo_get_model(), name));
    while (0 != intel_events[i].model_id) {
        if (perfexpert_cpuinfo_get_model() == intel_events[i].model_id) {
            j = 0;
            while (NULL != intel_events[i].events[j]) {
                if (0 == strcmp(intel_events[i].events[j], name)) {
                    return PERFEXPERT_TRUE;
                }
                j++;
            }
        }
        i++;
    }

    OUTPUT_VERBOSE((8, "event not available [%s]", name));

    return PERFEXPERT_FALSE;
}

#ifdef __cplusplus
}
#endif

// EOF
