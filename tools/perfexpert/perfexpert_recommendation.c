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
#include "config.h"
#include "perfexpert.h"
#include "perfexpert_output.h"
#include "perfexpert_fork.h"
#include "perfexpert_constants.h"

/* recommendation */
int recommendation(void) {
    char temp_str[8][BUFFER_SIZE];
    char *argv[6];
    test_t test;
    int rc;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Recommendations phase")));
    OUTPUT(("Selecting optimizations"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.txt", globals.stepdir, ANALYZER_METRICS);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_INPUT_FILE", temp_str[0], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.rec_count);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_REC_COUNT", temp_str[1], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_RECOMMENDER_DATABASE_FILE", globals.dbfile, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_COLORFUL", temp_str[2], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[7], BUFFER_SIZE);
    sprintf(temp_str[7], "%d", globals.verbose_level);
    if (0 != setenv("PERFEXPERT_RECOMMENDER_VERBOSE_LEVEL", temp_str[7], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[6], BUFFER_SIZE);
    sprintf(temp_str[6], "%d", (int)getpid());
    if (0 != setenv("PERFEXPERT_RECOMMENDER_PID", temp_str[6], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s", RECOMMENDER_PROGRAM);
    argv[0] = temp_str[3];
    argv[1] = "--automatic";
    argv[2] = globals.stepdir;
    argv[3] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/%s.txt", globals.stepdir, RECOMMENDER_METRICS);
    argv[4] = temp_str[4];
    argv[5] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line (WTF?) */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    if (0 == globals.verbose_level) {
        bzero(temp_str[5], BUFFER_SIZE);
        sprintf(temp_str[5], "%s/%s.output", globals.stepdir,
                RECOMMENDER_METRICS);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;

    /* Run! (to generate recommendation metrics for code transformer) */
    if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
        rc = fork_and_wait(&test, argv);
        if (PERFEXPERT_ERROR == rc) {
            return PERFEXPERT_ERROR;
        }
    }

    /* Work some arguments... */
    argv[1] = "--output";
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/%s.txt", globals.stepdir, RECOMMENDER_REPORT);
    argv[2] = temp_str[4];
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 2; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    if (0 == globals.verbose_level) {
        bzero(temp_str[5], BUFFER_SIZE);
        sprintf(temp_str[5], "%s/%s.output", globals.stepdir,
                RECOMMENDER_REPORT);
        test.output = temp_str[5];
    } else {
        test.output = NULL;
    }
    test.input = NULL;
    test.info = NULL;


    /* Run! (to generate recommendation report) */
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
