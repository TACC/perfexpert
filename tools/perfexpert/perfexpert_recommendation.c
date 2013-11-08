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

/* Tools headers */
#include "perfexpert_types.h"
#include "perfexpert.h"

/* PerfExpert common headers */
#include "common/perfexpert_output.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_constants.h"

/* recommendation */
int recommendation(void) {
    char temp_str[7][MAX_BUFFER_SIZE];
    char *argv[2];
    test_t test;
    int rc;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Recommendations phase")));
    OUTPUT(("%s", _YELLOW("Selecting recommendations for optimization")));

    bzero(temp_str[1], MAX_BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_COLORFUL", temp_str[1], 0)) {
        goto ERROR;
    }

    bzero(temp_str[2], MAX_BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.verbose);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL", temp_str[2], 0)) {
        goto ERROR;
    }

    bzero(temp_str[3], MAX_BUFFER_SIZE);
    sprintf(temp_str[3], "%llu", globals.unique_id);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_UID", temp_str[3], 0)) {
        goto ERROR;
    }

    bzero(temp_str[4], MAX_BUFFER_SIZE);
    sprintf(temp_str[4], "%d", globals.rec_count);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_REC_COUNT", temp_str[4], 0)) {
        goto ERROR;
    }

    bzero(temp_str[5], MAX_BUFFER_SIZE);
    sprintf(temp_str[5], "%s/%s", globals.stepdir, RECOMMENDER_REPORT);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE", temp_str[5], 1)) {
        goto ERROR;
    }

    bzero(temp_str[6], MAX_BUFFER_SIZE);
    sprintf(temp_str[6], "%s/%s", globals.stepdir, RECOMMENDER_METRICS);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_METRICS_FILE", temp_str[6], 1)) {
        goto ERROR;
    }

    if (0 != setenv("PERFEXPERT_RECOMMENDER_DATABASE", globals.dbfile, 0)) {
        goto ERROR;
    }

    if (0 != setenv("PERFEXPERT_RECOMMENDER_WORKDIR", globals.stepdir, 1)) {
        goto ERROR;
    }

    /* Arguments to run analyzer */
    argv[0] = RECOMMENDER_PROGRAM;
    argv[1] = NULL;

    /* The super-ninja test sctructure */
    bzero(temp_str[0], MAX_BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s", globals.stepdir, RECOMMENDER_OUTPUT);
    test.output = temp_str[0];
    test.input = NULL;
    test.info = RECOMMENDER_REPORT;

    /* run_and_fork_and_pray */
    return fork_and_wait(&test, argv);

    /* Error handling */
    ERROR:
    OUTPUT(("%s", _ERROR("unable to set environment variable")));
    return PERFEXPERT_ERROR;
}

#ifdef __cplusplus
}
#endif

// EOF
