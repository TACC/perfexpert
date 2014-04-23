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

/* Modules headers */
#include "vtune.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_time.h"
#include "install_dirs.h"

/* run_amplxe_cl */
int run_amplxecl(void) {
    struct timespec time_start, time_end, time_diff;
    vtune_event_t *event = NULL, *t = NULL;
    char events[MAX_BUFFER_SIZE];
    char *argv[MAX_ARGUMENTS_COUNT];
    int argc = 0, i = 0, rc;
    test_t test;

    /* Set the events we want to cVTune collect */
    bzero(events, MAX_BUFFER_SIZE);
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
        strcat(events, event->name);
        strcat(events, ",");
    }
    events[strlen(events) - 1] = '\0';

    if (0 != setenv("PERFEXPERT_VTUNE_EVENTS", events, 0)) {
        OUTPUT(("%s", _ERROR("unable to set environment variable")));
        return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE((8, "   %s %s", _YELLOW("events:"), events));

    /* Run the BEFORE program */
    if (NULL != my_module_globals.before[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/before.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.before[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.before)) {
            OUTPUT(("   %s", _RED("'before' command returns non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Create the command line: 5 steps */
    /* Step 1: add the PREFIX */
    i = 0;
    while (NULL != my_module_globals.prefix[i]) {
        argv[argc] = my_module_globals.prefix[i];
        argc++;
        i++;
    }

    /* Step 2: arguments to run VTune */
    argv[argc] = VTUNE_CL_BIN;
    argc++;
    argv[argc] = "-allow-multiple-runs";
    argc++;
    argv[argc] = "-collect-with";
    argc++;
    argv[argc] = "runsa";
    argc++;
    // argv[argc] = "runsa-knc";
    // argc++;
    argv[argc] = "-knob";
    argc++;
    argv[argc] = "event-config=$PERFEXPERT_VTUNE_EVENTS";
    argc++;
    argv[argc] = "--";
    argc++;

    /* Step 3: add the program and... */
    argv[argc] = globals.program_full;
    argc++;

    /* Step 4: add program's arguments */
    i = 0;
    while (NULL != globals.program_argv[i]) {
        argv[argc] = globals.program_argv[i];
        argc++;
        i++;
    }

    /* Step 5: set 'the last of the Mohicans' */
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
    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 14));
    sprintf(test.output, "%s/vtune.output", globals.moduledir);
    test.input = my_module_globals.inputfile;
    test.info = globals.program;

    /* fork_and_wait_and_pray */
    rc = perfexpert_fork_and_wait(&test, (char **)argv);
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    /* Evaluate results if required to */
    if (PERFEXPERT_FALSE == my_module_globals.ignore_return_code) {
        switch (rc) {
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_ERROR:
                OUTPUT(("%s (return code: %d) Usually, this means that an error"
                    " happened during the program execution. To see the program"
                    "'s output, check the content of this file: [%s]. If you "
                    "want to PerfExpert ignore the return code next time you "
                    "run this program, set the 'return-code' option for the "
                    "Vtune module. See 'perfepxert -H vtune' for details.",
                    _ERROR("the target program returned non-zero"), rc,
                    test.output));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
                break;

            default:
                break;
        }
    }

    /* Calculate and display runtime */
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    perfexpert_time_diff(&time_diff, &time_start, &time_end);
    OUTPUT(("   [1/1] %lld.%.9ld seconds (includes measurement overhead)",
        (long long)time_diff.tv_sec, time_diff.tv_nsec));
    PERFEXPERT_DEALLOC(test.output);

    /* Run the AFTER program */
    if (NULL != my_module_globals.after[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/after.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.after[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.after)) {
            OUTPUT(("%s", _RED("'after' command return non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
