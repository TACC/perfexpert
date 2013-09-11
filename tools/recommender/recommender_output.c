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

/* System standard headers */
#include <stdio.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_output.h"

/* output_recommendations */
int output_recommendations(void *var, int count, char **val, char **names) {
    int *rc = (int *)var;

    OUTPUT_VERBOSE((7, "   %s (%s)", _GREEN("recommendation found"), val[2]));
    
    if (NULL == globals.workdir) {
        /* Pretty print for the user */
        fprintf(globals.outputfile_FP,
            "#\n# Here is a possible recommendation for this code segment\n");
        fprintf(globals.outputfile_FP, "#\nID: %s\n", val[2]);
        fprintf(globals.outputfile_FP, "Description: %s\n", val[0]);
        fprintf(globals.outputfile_FP, "Reason: %s\n", val[1]);
        fprintf(globals.outputfile_FP, "Code example:\n%s\n", val[3]);
    } else {
        /* PerfExpert output */
        fprintf(globals.outputfile_FP, "recommender.recommendation_id=%s\n",
                val[2]);
    }

    *rc = PERFEXPERT_SUCCESS;
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
