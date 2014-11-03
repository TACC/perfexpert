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
#include <papi.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_tools.h"
#include "hpctoolkit_path.h"

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

/* run_hpcstruct */
int run_hpcstruct(void) {
    int rc = PERFEXPERT_SUCCESS, i = 0;
    char *argv[5];
    test_t test;

    /* Arguments to run hpcstruct */
    argv[0] = HPCSTRUCT;
    argv[1] = "--output";
    PERFEXPERT_ALLOC(char, argv[2],
        (strlen(globals.moduledir) + strlen(globals.program) + 23));
    sprintf(argv[2], "%s/%s.hpcstruct", globals.moduledir, globals.program);
    argv[3] = globals.program_full;
    argv[4] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output,
        (strlen(globals.moduledir) + strlen(HPCSTRUCT) + 20));
    sprintf(test.output, "%s/%s.output", globals.moduledir, HPCSTRUCT);
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    rc = perfexpert_fork_and_wait(&test, argv);

    /* FRee memory */
    PERFEXPERT_DEALLOC(argv[2]);
    PERFEXPERT_DEALLOC(test.output);

    return rc;
}

/* run_hpcrun */
int run_hpcrun(void) {
    int experiment_count = 0, experiment_total = 0, event_count = 0, i = 0, rc;
    struct timespec time_start, time_end, time_diff;
    hpctoolkit_event_t *event = NULL, *t = NULL;
    perfexpert_list_t experiments;
    char *str = NULL;
    experiment_t *e;
    test_t test;

    /* Initiate the list of experiments by adding the events */
    perfexpert_list_construct(&experiments);
    OUTPUT_VERBOSE((10, "there will be %d events/run", papi_max_events() - 1));

    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
        if (0 < event_count) {
            /* Add event */
            e->argv[e->argc] = "--event";
            e->argc++;
            PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(event->name) + 15);
            sprintf(e->argv[e->argc], "%s:%d", event->name,
                papi_get_sampling_rate(event->name));
            e->argc++;

            event_count--;
            continue;
        }

        /* Create a new experiment */
        PERFEXPERT_ALLOC(experiment_t, e, sizeof(experiment_t));
        perfexpert_list_item_construct((perfexpert_list_item_t *)e);
        perfexpert_list_append(&experiments, (perfexpert_list_item_t *)e);
        e->argc = 0;

        /* Add PREFIX to argv */
        i = 0;
        while (NULL != my_module_globals.prefix[i]) {
            e->argv[e->argc] = my_module_globals.prefix[i];
            e->argc++;
            i++;
        }

        /* Arguments to run hpcrun */
        e->argv[e->argc] = HPCRUN;
        e->argc++;
        e->argv[e->argc] = "--output";
        e->argc++;
        PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(globals.moduledir)+15);
        sprintf(e->argv[e->argc],"%s/measurements", globals.moduledir);
        e->argc++;

        event_count = papi_max_events() - 3;

        /* Set total number of instructions on every experiment */
        if (NULL == myself_module.total_inst_counter) {
            OUTPUT(("%s", _ERROR("total # of instructions counter not set")));
            return PERFEXPERT_ERROR;
        } else {
            PERFEXPERT_ALLOC(char, str,
                (strlen(myself_module.total_inst_counter) + 1));
            strcpy(str, myself_module.total_inst_counter);
            perfexpert_string_replace_char(str, '.', ':');
        }

        e->argv[e->argc] = "--event";
        e->argc++;
        PERFEXPERT_ALLOC(char, e->argv[e->argc], (strlen(str) + 15));
        sprintf(e->argv[e->argc], "%s:%d", str, papi_get_sampling_rate(str));
        e->argc++;

        /* Add event */
        e->argv[e->argc] = "--event";
        e->argc++;
        PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(event->name) + 15);
        sprintf(e->argv[e->argc], "%s:%d", event->name,
            papi_get_sampling_rate(event->name));
        e->argc++;

        continue;
    }

    experiment_total = perfexpert_list_get_size(&experiments);

    /* For each experiment... */
    OUTPUT_VERBOSE((5, "%s", _YELLOW("running experiments")));
    while (0 < perfexpert_list_get_size(&experiments)) {
        e = (experiment_t *)perfexpert_list_get_first(&experiments);
        experiment_count++;

        /* Run the BEFORE program */
        if (NULL != my_module_globals.before[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir)+20));
            sprintf(test.output, "%s/before.%d.output", globals.moduledir,
                experiment_count);
            test.input = NULL;
            test.info = my_module_globals.before[0];

            if (0 != perfexpert_fork_and_wait(&test,
                (char **)my_module_globals.before)) {
                OUTPUT(("   %s", _RED("'before' command returns non-zero")));
            }
            PERFEXPERT_DEALLOC(test.output);
        }

        /* Ok, now we have to add the program and... */
        e->argv[e->argc] = globals.program_full;
        e->argc++;

        /* ...and the program arguments */
        i = 0;
        while (NULL != globals.program_argv[i]) {
            e->argv[e->argc] = globals.program_argv[i];
            e->argc++;
            i++;
        }

        /* The last of the Mohicans */
        e->argv[e->argc] = NULL;

        /* The super-ninja test sctructure */
        PERFEXPERT_ALLOC(char, e->test.output,
            (strlen(globals.moduledir) + strlen(HPCRUN) + 25));
        sprintf(e->test.output, "%s/%s.%d.output", globals.moduledir, HPCRUN,
            experiment_count);
        e->test.input = my_module_globals.inputfile;
        e->test.info = globals.program;

        /* Not using OUTPUT_VERBOSE because I want only one line */
        if (4 <= globals.verbose) {
            printf("%s %s", PROGRAM_PREFIX, _YELLOW("command line:"));
            for (i = 0; i < e->argc; i++) {
                printf(" %s", e->argv[i]);
            }
            printf("\n");
        }

        /* fork_and_wait_and_pray and calculate and display runtime */
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        rc = perfexpert_fork_and_wait(&(e->test), (char **)e->argv);
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        perfexpert_time_diff(&time_diff, &time_start, &time_end);
        OUTPUT(("   [%d/%d] %lld.%.9ld seconds (includes measurement overhead)",
            experiment_count, experiment_total, (long long)time_diff.tv_sec,
            time_diff.tv_nsec));

        /* Evaluate results if required to */
        if (PERFEXPERT_FALSE == my_module_globals.ignore_return_code) {
            switch (rc) {
                case PERFEXPERT_FAILURE:
                case PERFEXPERT_ERROR:
                    OUTPUT(("%s (return code: %d) Usually, this means that an "
                        "error happened during the program execution. To see "
                        "the program's output, check the content of this file: "
                        "[%s]. If you want to PerfExpert ignore the return code"
                        " next time you run this program, set the 'return-code'"
                        " option for the HPCToolkit module. See 'perfepxert -H "
                        "hpctoolkit' for details.",
                        _ERROR("the target program returned non-zero"), rc,
                        e->test.output));
                    return PERFEXPERT_ERROR;

                case PERFEXPERT_SUCCESS:
                    OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
                    break;

                default:
                    break;
            }
        }

        /* Run the AFTER program */
        if (NULL != my_module_globals.after[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir)+20));
            sprintf(test.output, "%s/after.%d.output", globals.moduledir,
                experiment_count);
            test.input = NULL;
            test.info = my_module_globals.after[0];

            if (0 != perfexpert_fork_and_wait(&test,
                (char **)my_module_globals.after)) {
                OUTPUT(("%s", _RED("'after' command return non-zero")));
            }
            PERFEXPERT_DEALLOC(test.output);
        }

        /* Remove experiment from list */
        perfexpert_list_remove_item(&experiments,(perfexpert_list_item_t *)(e));
        PERFEXPERT_DEALLOC(e->test.output);
        PERFEXPERT_DEALLOC(e);
        e = (experiment_t *)perfexpert_list_get_first(&experiments);
    }
    return rc;
}

/* run_hpcprof */
int run_hpcprof(char **file) {
    int rc = PERFEXPERT_SUCCESS, i = 0;
    char *argv[9];
    test_t test;

    /* Arguments to run hpcprof */
    argv[0] = HPCPROF;
    argv[1] = "--force-metric";
    argv[2] = "--metric=thread";
    argv[3] = "--struct";
    PERFEXPERT_ALLOC(char, argv[4],
        (strlen(globals.moduledir) + strlen(globals.program) + 23));
    sprintf(argv[4], "%s/%s.hpcstruct", globals.moduledir, globals.program);
    argv[5] = "--output";
    PERFEXPERT_ALLOC(char, argv[6], (strlen(globals.moduledir) + 21));
    sprintf(argv[6], "%s/database", globals.moduledir);
    PERFEXPERT_ALLOC(char, argv[7], (strlen(globals.moduledir) + 25));
    sprintf(argv[7], "%s/measurements", globals.moduledir);
    argv[8] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 8; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output,
        (strlen(globals.moduledir) + strlen(HPCPROF) + 20));
    sprintf(test.output, "%s/%s.output", globals.moduledir, HPCPROF);
    test.input = NULL;
    test.info = globals.program;

    /* fork_and_wait_and_pray */
    rc = perfexpert_fork_and_wait(&test, argv);

    PERFEXPERT_ALLOC(char, *file, strlen(argv[6]) + 16);
    sprintf(*file, "%s/experiment.xml", argv[6]);

    /* Free memory */
    PERFEXPERT_DEALLOC(argv[4]);
    PERFEXPERT_DEALLOC(argv[6]);
    PERFEXPERT_DEALLOC(argv[7]);
    PERFEXPERT_DEALLOC(test.output);

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
