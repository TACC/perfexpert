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
#include <time.h>

/* PerfExpert headers */
#include "perfexpert.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_fork.h"
#include "perfexpert_time.h"
#include "perfexpert_util.h"
#include "perfexpert_vtune.h"

/* measurements_hpctoolkit */
int measurements_vtune(void) {
    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_exec(globals.program)) {
        return PERFEXPERT_ERROR;
    }
    /* Run Intel VTune Amplifier XE */
    if (PERFEXPERT_SUCCESS != run_amplxecl()) {
        OUTPUT(("%s", _ERROR("Error: unable to run amplxe-cl")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* run_amplxe-cl */
int run_amplxecl(void) {
    struct timespec time_start, time_end, time_diff;
    FILE *exp_file_FP = NULL;
    char *exp_file = NULL;
    char *local_prefix = NULL;
    char buffer[BUFFER_SIZE];
    char events[BUFFER_SIZE];
    char *argv[PARAM_SIZE];
    int argc = 0;
    int count = 0;
    test_t test;

    /* Open experiment file (it is a list of arguments which I use to run) */
    PERFEXPERT_ALLOC(char, exp_file,
        (strlen(PERFEXPERT_ETCDIR) + strlen(VTUNE_EXPERIMENT_FILE) + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR, VTUNE_EXPERIMENT_FILE);

    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), exp_file));
        PERFEXPERT_DEALLOC(exp_file);
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(exp_file);

    if (NULL != globals.prefix) {
        PERFEXPERT_ALLOC(char, local_prefix, (strlen(globals.prefix) + 1));
        sprintf(local_prefix, "%s", globals.prefix);
    }

    bzero(events, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, exp_file_FP)) {
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Accumulate the event */
        strcat(events, buffer);
    }
    /* Close file */
    fclose(exp_file_FP);

    /* Set events */
    if (0 != setenv("VTUNE_AMPLIFIER_EVENTS", events, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Run the BEFORE program */
    if (NULL != globals.before) {
        argv[0] = globals.before;
        argv[1] = NULL;

        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
        sprintf(test.output, "%s/before.output", globals.stepdir);
        test.input = NULL;
        test.info = globals.before;

        if (0 != fork_and_wait(&test, (char **)argv)) {
            OUTPUT(("   %s [%s]", _RED("error running"), globals.before));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Add PREFIX to argv */
    if (NULL != globals.prefix) {
        argv[argc] = strtok(local_prefix, " ");
        argc++;
        while (argv[argc] = strtok(NULL, " ")) {
            argc++;
        }
    }

    /* Arguments to run VTune */
    argv[argc] = VTUNE_AMPLIFIER;
    argc++;
    argv[argc] = "-allow-multiple-runs";
    argc++;
    argv[argc] = "-collect-with";
    argc++;
    argv[argc] = "runsa-knc";
    argc++;
    argv[argc] = "-knob";
    argc++;
    argv[argc] = "runsa";
    argc++;
    argv[argc] = "event-config=$VTUNE_AMPLIFIER_EVENTS";
    argc++;

    /* Now we add the program and... */
    argv[argc] = globals.program_full;
    argc++;

    /* ...and it's arguments */
    while (NULL != globals.program_argv[count]) {
        argv[argc] = globals.program_argv[count];
        argc++;
        count++;
    }
    argv[argc] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (count = 0; count < argc; count++) {
            printf(" %s", argv[count]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
    sprintf(test.output, "%s/vtune.%d.output", globals.stepdir);
    test.input  = NULL;
    test.info   = globals.program;

    /* Run program and test return code (should I really test it?) */
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    switch (fork_and_wait(&test, (char **)argv)) {
        case PERFEXPERT_ERROR:
            OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
            PERFEXPERT_DEALLOC(local_prefix);
            PERFEXPERT_DEALLOC(test.output);
            return PERFEXPERT_ERROR;

        case PERFEXPERT_FAILURE:
            OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
            break;

        case PERFEXPERT_SUCCESS:
             OUTPUT_VERBOSE((7, "   [ %s  ]", _BOLDGREEN("OK")));
            break;

        default:
            break;
    }
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    perfexpert_time_diff(&time_diff, &time_start, &time_end);
    OUTPUT(("   run in %lld.%.9ld seconds", (long long)time_diff.tv_sec,
        time_diff.tv_nsec));
    PERFEXPERT_DEALLOC(test.output);
    
    /* Run the AFTER program */
    if (NULL != globals.after) {
        argv[0] = globals.after;
        argv[1] = NULL;

        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
        sprintf(test.output, "%s/after.output", globals.stepdir);
        test.input  = NULL;
        test.info   = globals.after;

        if (0 != fork_and_wait(&test, (char **)argv)) {
            OUTPUT(("   %s [%s]", _RED("error running"), globals.after));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Free memory */
    PERFEXPERT_DEALLOC(local_prefix);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
