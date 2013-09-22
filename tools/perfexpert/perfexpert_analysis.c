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
#include "perfexpert_util.h"
#include "perfexpert_fork.h"
#include "perfexpert_constants.h"
#include "install_dirs.h"

/* analysis */
int analysis(void) {
    char *argv[2];
    char temp_str[10][BUFFER_SIZE];
    test_t test;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Analysis phase")));
    OUTPUT(("Analysing measurements"));

    /* Set some environment variables to avoid working arguments */
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%d", globals.verbose);
    if (0 != setenv("PERFEXPERT_ANALYZER_VERBOSE_LEVEL", temp_str[1], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%d", globals.colorful);
    if (0 != setenv("PERFEXPERT_ANALYZER_COLORFUL", temp_str[2], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s/%s", PERFEXPERT_ETCDIR, LCPI_FILE);
    if (0 != setenv("PERFEXPERT_ANALYZER_LCPI_FILE", temp_str[3], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/%s", PERFEXPERT_ETCDIR, MACHINE_FILE);
    if (0 != setenv("PERFEXPERT_ANALYZER_MACHINE_FILE", temp_str[4], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[5], BUFFER_SIZE);
    sprintf(temp_str[5], "%f", globals.threshold);
    if (0 != setenv("PERFEXPERT_ANALYZER_THRESHOLD", temp_str[5], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[6], BUFFER_SIZE);
    sprintf(temp_str[6], "%s/%s", globals.stepdir,
        PERFEXPERT_TOOL_HPCTOOLKIT_PROFILE_FILE);
    if (0 != setenv("PERFEXPERT_ANALYZER_INPUT_FILE", temp_str[6], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[7], BUFFER_SIZE);
    sprintf(temp_str[7], "%s/%s", globals.stepdir, ANALYZER_REPORT);
    if (0 != setenv("PERFEXPERT_ANALYZER_OUTPUT_FILE", temp_str[7], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    // TODO: this will change
    bzero(temp_str[8], BUFFER_SIZE);
    sprintf(temp_str[8], "%s", "hpctoolkit");
    if (0 != setenv("PERFEXPERT_ANALYZER_TOOL", temp_str[8], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str[9], BUFFER_SIZE);
    sprintf(temp_str[9], "%s/%s", globals.stepdir, ANALYZER_METRICS);
    if (0 != setenv("PERFEXPERT_ANALYZER_METRICS_FILE", temp_str[9], 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    if (0 != setenv("PERFEXPERT_ANALYZER_WORKDIR", globals.stepdir, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Arguments to run analyzer */
    argv[0] = ANALYZER_PROGRAM;
    argv[1] = NULL;

    /* The super-ninja test sctructure */
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s", globals.stepdir, ANALYZER_OUTPUT);
    test.output = temp_str[0];
    test.input  = NULL;
    test.info   = PERFEXPERT_TOOL_HPCTOOLKIT_PROFILE_FILE;

    /* run_and_fork_and_pray */
    return fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
