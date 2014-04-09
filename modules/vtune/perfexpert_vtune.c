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
        OUTPUT(("%s", _ERROR("unable to run amplxe-cl")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* run_amplxe-cl */
int run_amplxecl(void) {
    struct timespec time_start, time_end, time_diff;
    FILE *exp_file_FP = NULL;
    char *exp_file = NULL;
    char buffer[BUFFER_SIZE];
    char events[BUFFER_SIZE];
    char *argv[PARAM_SIZE];
    int argc = 0, i = 0;
    test_t test;

    /* Open experiment file (it is a list of arguments which I use to run) */
    PERFEXPERT_ALLOC(char, exp_file,
        (strlen(PERFEXPERT_ETCDIR) + strlen(VTUNE_EXPERIMENT_FILE) + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR, VTUNE_EXPERIMENT_FILE);

    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), exp_file));
        PERFEXPERT_DEALLOC(exp_file);
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(exp_file);

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
        OUTPUT(("%s", _ERROR("unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }

    /* Run the BEFORE program */
    if (NULL != globals.before[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 15));
        sprintf(test.output, "%s/before.output", globals.stepdir);
        test.input = NULL;
        test.info = globals.before[0];

        if (0 != fork_and_wait(&test, (char **)globals.before)) {
            OUTPUT(("   %s", _RED("error running 'before' command")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Add PREFIX to argv */
    i = 0;
    while (NULL != globals.prefix[i]) {
        argv[argc] = globals.prefix[i];
        argc++;
        i++;
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
    i = 0;
    while (NULL != globals.program_argv[i]) {
        argv[argc] = globals.program_argv[i];
        argc++;
        i++;
    }

    /* The last of the Mohicans */
    argv[argc] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
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
    if (NULL != globals.after[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 14));
        sprintf(test.output, "%s/after.output", globals.stepdir);
        test.input = NULL;
        test.info = globals.after[0];

        if (0 != fork_and_wait(&test, (char **)globals.after)) {
            OUTPUT(("   %s", _RED("error running 'after' command")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
