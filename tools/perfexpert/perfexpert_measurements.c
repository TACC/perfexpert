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

/* PerfExpert headers */
#include "perfexpert.h"
#include "perfexpert_measurements.h"
#include "perfexpert_output.h"

/* Measurements tools definition */
tool_t tools[] = {
    {
        "hpctoolkit",
        &measurements_hpctoolkit,
        HPCTOOLKIT_PROFILE_FILE
    }, {
        "vtune",
        &measurements_vtune,
        VTUNE_PROFILE_FILE
    }, {NULL, NULL, NULL}
};

/* measurements */
int measurements(void) {
    int i = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Measurements phase")));

    /* Find the measurement function for this tool */
    while (NULL != tools[i].name) {
        if (0 == strcmp(globals.tool, tools[i].name)) {
            OUTPUT(("%s [%s] using %s", _YELLOW("Running"), globals.program,
                globals.tool));
            /* Call the measurement function for this tool */
            return (*tools[i].function)();
        }
        i++;
    }

    OUTPUT(("%s [%s]", _ERROR("Error: unknown measurement tool"),
        globals.tool));
    return PERFEXPERT_ERROR;
}

#ifdef __cplusplus
}
#endif

// EOF
