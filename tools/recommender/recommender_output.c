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

/* PerfExpert headers */
#include "recommender.h"
#include "perfexpert_output.h"

/* output_header */
int output_header(segment_t *segment) {
    fprintf(globals.outputfile_FP,
        "#--------------------------------------------------\n");
    fprintf(globals.outputfile_FP, "# Recommendations for %s:%d\n",
        segment->filename, segment->line_number);
    fprintf(globals.outputfile_FP,
        "#--------------------------------------------------\n");

    if (NULL != globals.outputmetrics) {
        fprintf(globals.outputmetrics_FP, "%% recommendation for %s:%d\n",
            segment->filename, segment->line_number);
        fprintf(globals.outputmetrics_FP, "code.filename=%s\n",
            segment->filename);
        fprintf(globals.outputmetrics_FP, "code.line_number=%d\n",
            segment->line_number);
        fprintf(globals.outputmetrics_FP, "code.type=%d\n", segment->type);
        fprintf(globals.outputmetrics_FP, "code.function_name=%s\n",
            segment->function_name);
        fprintf(globals.outputmetrics_FP, "code.loopdepth=%d\n",
            segment->loopdepth);
        fprintf(globals.outputmetrics_FP, "code.rowid=%d\n", segment->rowid);
    }
    return PERFEXPERT_SUCCESS;
}

/* output_recommendations */
int output_recommendations(void *var, int count, char **val, char **names) {
    int *rc = (int *)var;

    OUTPUT_VERBOSE((7, "         Recom=%s", val[2]));

    /* Pretty print for the user */
    fprintf(globals.outputfile_FP,
        "#\n# Here is a possible recommendation for this code segment\n");
    fprintf(globals.outputfile_FP, "#\nID: %s\n", val[2]);
    fprintf(globals.outputfile_FP, "Description: %s\n", val[0]);
    fprintf(globals.outputfile_FP, "Reason: %s\n", val[1]);
    fprintf(globals.outputfile_FP, "Code example:\n%s\n", val[3]);

    /* Output metrics */
    if (NULL != globals.outputmetrics) {
        fprintf(globals.outputmetrics_FP, "recommender.recommendation_id=%s\n",
            val[2]);
    }

    *rc = PERFEXPERT_SUCCESS;
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
