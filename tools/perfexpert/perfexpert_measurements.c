/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert headers */
#include "config.h"
#include "perfexpert.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_constants.h"

/* measurements */
int measurements(void) {
    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Measurements phase")));
    OUTPUT(("Running [%s]", globals.program));

    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists_and_is_exec(
        globals.program)) {
        return PERFEXPERT_ERROR;
    }
    /* Create the program structure file */
    if (PERFEXPERT_SUCCESS != run_hpcstruct()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcstruct")));
        return PERFEXPERT_ERROR;
    }
    /* Collect measurements */
    if (NULL == globals.knc) {
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
    if (PERFEXPERT_SUCCESS != run_hpcprof()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcprof")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
