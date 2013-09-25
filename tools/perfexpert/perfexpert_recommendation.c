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
#include <stdlib.h>
#include <string.h>

/* PerfExpert headers */
#include "perfexpert.h"
#include "perfexpert_output.h"
#include "perfexpert_fork.h"
#include "perfexpert_constants.h"

/* recommendation */
int recommendation(void) {
    char temp_str[8][BUFFER_SIZE];
    char *argv[2];
    test_t test;
    int rc;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Recommendations phase")));
    OUTPUT(("Selecting optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_COLORFUL", temp_str[1], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.verbose);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL", temp_str[2], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%d", (int)getpid());
    if (0 != setenv("PERFEXPERT_RECOMMENDER_PID", temp_str[3], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%d", globals.rec_count);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_REC_COUNT", temp_str[4], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[5], BUFFER_SIZE);
    sprintf(temp_str[5], "%s/%s", globals.stepdir, ANALYZER_METRICS);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_INPUT_FILE", temp_str[5], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[6], BUFFER_SIZE);
    sprintf(temp_str[6], "%s/%s", globals.stepdir, RECOMMENDER_REPORT);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_OUTPUT_FILE", temp_str[6], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[7], BUFFER_SIZE);
    sprintf(temp_str[7], "%s/%s", globals.stepdir, RECOMMENDER_METRICS);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_METRICS_FILE", temp_str[7], 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE", globals.dbfile,
        0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_WORKDIR", globals.stepdir, 1)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    argv[0] = RECOMMENDER_PROGRAM;
    argv[1] = NULL;

    /* The super-ninja test sctructure */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s", globals.stepdir, RECOMMENDER_OUTPUT);
    test.output = temp_str[0];
    test.input = NULL;
    test.info = temp_str[5];

    /* run_and_fork_and_pray */
    rc = fork_and_wait(&test, argv);
    if (PERFEXPERT_ERROR == rc) {
        return PERFEXPERT_ERROR;
    }

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
