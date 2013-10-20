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
#include "perfexpert_output.h"
#include "perfexpert_module.h"

/* measurements */
int measurements(void) {
    OUTPUT_VERBOSE((4, "%s", _BLUE("Measurements phase")));
    OUTPUT(("%s [%s]", _YELLOW("Collecting measurements"), globals.tool));

    if (PERFEXPERT_SUCCESS !=
        perfexpert_load_module(globals.tool, &(globals.toolmodule))) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to load tool module"),
            globals.tool));
        return PERFEXPERT_ERROR;
    }

    /* Just to be sure... */
    if (NULL == globals.toolmodule.measurements) {
        OUTPUT(("%s [%s]",
            _ERROR("Error: tool module does not implement measurements()"),
            globals.tool));
        return PERFEXPERT_ERROR;
    }

    /* Call the measurement function for this tool */
    return globals.toolmodule.measurements();
}

#ifdef __cplusplus
}
#endif

// EOF
