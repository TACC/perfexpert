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
#include "perfexpert_util.h"
#include "perfexpert_fork.h"
#include "perfexpert_constants.h"

/* analysis */
int analysis(void) {
    char experiment[BUFFER_SIZE];
    char *argv[6];
    char temp_str[3][BUFFER_SIZE];
    test_t test;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Analysis phase")));
    OUTPUT(("Analysing measurements"));

    /* First of all, does the file exist? */
    bzero(experiment, BUFFER_SIZE);
    sprintf(experiment, "%s/database/experiment.xml", globals.stepdir);
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(experiment)) {
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s", ANALYZER_PROGRAM);
    argv[0] = temp_str[0];
    argv[1] = "--recommend";
    argv[2] = "--opttran";
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%f", globals.threshold);
    argv[3] = temp_str[1];
    argv[4] = experiment;
    argv[5] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/%s.txt", globals.stepdir, ANALYZER_METRICS);
    test.output = temp_str[2];
    test.input  = NULL;
    test.info   = experiment;

    /* Run! (to generate analysis generate metrics for recommender) */
    if (PERFEXPERT_SUCCESS != fork_and_wait(&test, argv)) {
        return PERFEXPERT_ERROR;
    }

    /* Work some arguments... */
    argv[1] = argv[3];
    argv[2] = argv[4];
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
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/%s.txt", globals.stepdir, ANALYZER_REPORT);
    test.output = temp_str[2];
    test.input  = NULL;
    test.info   = experiment;

    /* Run! (to generate analysis report) */
    return fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
