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

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_list.h"
#include "perfexpert_constants.h"

/* vtune_parse_file */
int vtune_parse_file(const char *file, perfexpert_list_t *profiles) {
    return PERFEXPERT_SUCCESS;

    OUTPUT_VERBOSE((2, "   %s [VTune Amplifier XE experiment DB version ??]",
        _BLUE("Loading profiles")));
}

#ifdef __cplusplus
}
#endif

// EOF
