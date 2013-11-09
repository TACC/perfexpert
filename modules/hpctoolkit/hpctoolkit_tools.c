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

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_tools.h"

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
        (strlen(globals.stepdir) + strlen(globals.program) + 23));
    sprintf(argv[2], "%s/hpctoolkit/%s.hpcstruct", globals.stepdir,
        globals.program);
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
        (strlen(globals.stepdir) + strlen(HPCSTRUCT) + 20));
    sprintf(test.output, "%s/hpctoolkit/%s.output", globals.stepdir, HPCSTRUCT);
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    rc = fork_and_wait(&test, argv);

    /* FRee memory */
    PERFEXPERT_DEALLOC(argv[2]);
    PERFEXPERT_DEALLOC(test.output);

    return rc;
}

/* run_hpcrun */
int run_hpcrun(void) {
    int experiment_count = 0, rc = PERFEXPERT_SUCCESS, i = 0, event_count = 0;
    struct timespec time_start, time_end, time_diff;
    hpctoolkit_event_t *event = NULL, *t = NULL;
    perfexpert_list_t experiments;
    experiment_t *e;
    test_t test;

    /* Initiate the list of experiments by adding the events */
    perfexpert_list_construct(&experiments);
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
        if (0 < event_count) {
            /* Add event */
            e->argv[e->argc] = "--event";
            e->argc++;
            PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(event->name) + 10);
            sprintf(e->argv[e->argc], "%s:%d", event->name,
                get_sampling_rate(event->name));
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
        PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(globals.stepdir) + 25);
        sprintf(e->argv[e->argc],"%s/hpctoolkit/measurements", globals.stepdir);
        e->argc++;

        /* Set PAPI_TOT_INS */
        e->argv[e->argc] = "--event";
        e->argc++;
        PERFEXPERT_ALLOC(char, e->argv[e->argc], (strlen("PAPI_TOT_INS") + 10));
        sprintf(e->argv[e->argc], "PAPI_TOT_INS:%d",
            get_sampling_rate(event->name));
        e->argc++;

        event_count = max_events() - 1;

        /* Add event */
        e->argv[e->argc] = "--event";
        e->argc++;
        PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(event->name) + 10);
        sprintf(e->argv[e->argc], "%s:%d", event->name,
            get_sampling_rate(event->name));
        e->argc++;

        event_count--;
        continue;
    }

    /* For each experiment... */
    OUTPUT_VERBOSE((5, "%s", _YELLOW("running experiments")));
    while (0 < perfexpert_list_get_size(&experiments)) {
        e = (experiment_t *)perfexpert_list_get_first(&experiments);
        experiment_count++;

        /* Run the BEFORE program */
        if (NULL != my_module_globals.before[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/before.%d.output", globals.stepdir,
                experiment_count);
            test.input = NULL;
            test.info = my_module_globals.before[0];

            if (0 != fork_and_wait(&test, (char **)my_module_globals.before)) {
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
            (strlen(globals.stepdir) + strlen(HPCRUN) + 25));
        sprintf(e->test.output, "%s/hpctoolkit/%s.%d.output", globals.stepdir,
            HPCRUN, experiment_count);
        e->test.input = my_module_globals.inputfile;
        e->test.info = globals.program;

        /* Not using OUTPUT_VERBOSE because I want only one line */
        if (8 <= globals.verbose) {
            printf("%s %s", PROGRAM_PREFIX, _YELLOW("command line:"));
            for (i = 0; i < e->argc; i++) {
                printf(" %s", e->argv[i]);
            }
            printf("\n");
        }

        /* (HPC)run program and test return code (should I really test it?) */
        clock_gettime(CLOCK_MONOTONIC, &time_start);

        switch (fork_and_wait(&(e->test), (char **)e->argv)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "[%s]", _BOLDYELLOW("ERROR")));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "[%s ]", _BOLDRED("FAIL")));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
                break;

            default:
                break;
        }

        clock_gettime(CLOCK_MONOTONIC, &time_end);
        perfexpert_time_diff(&time_diff, &time_start, &time_end);
        OUTPUT(("   [%d] %lld.%.9ld seconds (includes measurement overhead)",
            experiment_count, (long long)time_diff.tv_sec, time_diff.tv_nsec));

        /* Run the AFTER program */
        if (NULL != my_module_globals.after[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/after.%d.output", globals.stepdir,
                experiment_count);
            test.input = NULL;
            test.info = my_module_globals.after[0];

            if (0 != fork_and_wait(&test, (char **)my_module_globals.after)) {
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

/* run_hpcrun_knc */
int run_hpcrun_knc(void) {
    hpctoolkit_event_t *event = NULL, *t = NULL;
    int experiment_count = 0, count = 0, rc = PERFEXPERT_SUCCESS;
    char *script_file, *argv[4];
    FILE *script_file_FP;
    test_t test;

    /* If this command should run on the MIC, encapsulate it in a script */
    PERFEXPERT_ALLOC(char, script_file, (strlen(globals.stepdir) + 15));
    sprintf(script_file, "%s/knc_hpcrun.sh", globals.stepdir);

    if (NULL == (script_file_FP = fopen(script_file, "w"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), script_file));
        return PERFEXPERT_ERROR;
    }

    fprintf(script_file_FP, "#!/bin/sh");

    /* Fill the script file with all the experiments, before, and after */
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
        if (0 < count) {
            /* Add event */
            fprintf(script_file_FP, " --event %s:%d", event->name,
                get_sampling_rate(event->name));

            count--;
            continue;
        }

        /* Is this a new experiment, but not the first? */
        if (0 < experiment_count) {
            /* Add the program and the program arguments to experiment */
            fprintf(script_file_FP, " %s", globals.program_full);
            count = 0;
            while (NULL != globals.program_argv[count]) {
                fprintf(script_file_FP, " %s", globals.program_argv[count]);
                count++;
            }

            /* Add the AFTER program */
            if (NULL != my_module_globals.knc_after) {
                fprintf(script_file_FP, "\n\n# AFTER command\n");
                fprintf(script_file_FP, "%s", my_module_globals.knc_after);
            }
        }

        /* Add the BEFORE program */
        if (NULL != my_module_globals.knc_before) {
            fprintf(script_file_FP, "\n\n# BEFORE command\n");
            fprintf(script_file_FP, "%s", my_module_globals.knc_before);
        }

        fprintf(script_file_FP, "\n\n# HPCRUN (%d)\n", experiment_count);

        /* Add PREFIX */
        if (NULL != my_module_globals.knc_prefix) {
            fprintf(script_file_FP, "%s ", my_module_globals.knc_prefix);
        }

        /* Arguments to run hpcrun */
        fprintf(script_file_FP, "%s --output %s/measurements", HPCRUN,
            globals.stepdir);

        experiment_count++;
    }

    /* Add the program and the program arguments to experiment's */
    fprintf(script_file_FP, " %s", globals.program_full);
    count = 0;
    while (NULL != globals.program_argv[count]) {
        fprintf(script_file_FP, " %s", globals.program_argv[count]);
        count++;
    }

    /* Add the AFTER program */
    if (NULL != my_module_globals.knc_after) {
        fprintf(script_file_FP, "\n\n# AFTER command\n");
        fprintf(script_file_FP, "%s", my_module_globals.knc_after);
    }
    fprintf(script_file_FP, "\n\nexit 0\n\n# EOF\n");

    /* Close file and set mode */
    fclose(script_file_FP);
    if (-1 == chmod(script_file, S_IRWXU)) {
        OUTPUT(("%s (%s)", _ERROR("unable to set script mode"), script_file));
        PERFEXPERT_DEALLOC(script_file);
        return PERFEXPERT_ERROR;
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 19));
    sprintf(test.output, "%s/knc_hpcrun.output", globals.stepdir);
    test.input = NULL;
    test.info = globals.program;

    argv[0] = "ssh";
    argv[1] = my_module_globals.knc;
    argv[2] = script_file;
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s %s %s %s %s\n", PROGRAM_PREFIX, _YELLOW("command line:"),
            argv[0], argv[1], argv[2]);
    }

    /* Run program and test return code (should I really test it?) */
    switch (rc = fork_and_wait(&test, argv)) {
        case PERFEXPERT_ERROR:
            OUTPUT_VERBOSE((7, "[%s]", _BOLDYELLOW("ERROR")));
            break;

        case PERFEXPERT_FAILURE:
        case 255:
            OUTPUT_VERBOSE((7, "[%s ]", _BOLDRED("FAIL")));
            break;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
            break;

        default:
            OUTPUT_VERBOSE((7, "[UNKNO]"));
            break;
    }

    PERFEXPERT_DEALLOC(script_file);
    PERFEXPERT_DEALLOC(test.output);

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
        (strlen(globals.stepdir) + strlen(globals.program) + 23));
    sprintf(argv[4], "%s/hpctoolkit/%s.hpcstruct", globals.stepdir,
        globals.program);
    argv[5] = "--output";
    PERFEXPERT_ALLOC(char, argv[6], (strlen(globals.stepdir) + 21));
    sprintf(argv[6], "%s/hpctoolkit/database", globals.stepdir);
    PERFEXPERT_ALLOC(char, argv[7], (strlen(globals.stepdir) + 25));
    sprintf(argv[7], "%s/hpctoolkit/measurements", globals.stepdir);
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
        (strlen(globals.stepdir) + strlen(HPCPROF) + 20));
    sprintf(test.output, "%s/hpctoolkit/%s.output", globals.stepdir, HPCPROF);
    test.input = NULL;
    test.info = globals.program;

    /* fork_and_wait_and_pray */
    rc = fork_and_wait(&test, argv);

    PERFEXPERT_ALLOC(char, *file, strlen(argv[6]) + 16);
    sprintf(*file, "%s/experiment.xml", argv[6]);

    /* Free memory */
    PERFEXPERT_DEALLOC(argv[4]);
    PERFEXPERT_DEALLOC(argv[6]);
    PERFEXPERT_DEALLOC(argv[7]);
    PERFEXPERT_DEALLOC(test.output);

    return rc;
}

/* max_events */
static int max_events(void) {
    int papi_rc, event_set = PAPI_NULL, event_code, n = 0;
    hpctoolkit_event_t *e = NULL, *t = NULL;

    /* Find the maximum number of events per experiment */
    if (PAPI_OK != (papi_rc = PAPI_create_eventset(&event_set))) {
        OUTPUT(("%s [%s]", _ERROR("could not create PAPI event set"),
            PAPI_strerror(papi_rc)));
        return PERFEXPERT_FAILURE;
    }

    perfexpert_hash_iter_str(my_module_globals.events_by_name, e, t) {
        if (PAPI_OK != PAPI_event_name_to_code(e->name, &event_code)) {
            OUTPUT(("%s [%s]",
                _ERROR("event not available (name->code)"), e->name));
            return PERFEXPERT_FAILURE;
        }
        if (PAPI_OK != PAPI_add_event(event_set, event_code)) {
            return n;
        }
        n++;
    }
    return n;
}

/* get_prime */
static int get_prime(int start, int end) {
    int flag, i, j;

    for (i = start + 1; i <= end; i++) {
        flag = 0;
        for (j = 2; j <= i / 2; j++) {
            if (0 == i%j) {
                flag = 1;
                break;
            }
        }
        if (0 == flag) {
            return i;
        }
    }
    return -1;
}

/* get_sampling_rate */
static int get_sampling_rate(const char *name) {
    int event_code, cat = 0, rate = 0;
    PAPI_event_info_t info;

    static sample_t sample[] = {
        {99999999, 0, "PAPI_TOT_INS PAPI_TOT_CYC L1I_CYCLES_STALLED "},
        {9999999,  0, "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE:SSE_SCALAR_DOUBLE "
            "PAPI_LD_INS PAPI_L1_DCA PAPI_L1_ICA ICACHE ARITH:CYCLES_DIV_BUSY "
            "SSEX_UOPS_RETIRED:SCALAR_DOUBLE ARITH FP_COMP_OPS_EXE:SSE_FP "
            "FP_COMP_OPS_EXE:SSE_FP_PACKED FP_COMP_OPS_EXE:SSE_FP_SCALAR "
            "FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION RETIRED_SSE_OPERATIONS:ALL "
            "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION RETIRED_SSE_OPS:ALL "
            "SSEX_UOPS_RETIRED:PACKED_DOUBLE SSEX_UOPS_RETIRED:PACKED_SINGLE "
            "SSEX_UOPS_RETIRED:SCALAR_SINGLE FP_COMP_OPS_EXE:X87 PAPI_FDV_INS "
            "SIMD_COMP_INST_RETIRED:PACKED_DOUBLE SIMD_FP_256:PACKED_SINGLE "
            "SIMD_COMP_INST_RETIRED:PACKED_SINGLE SIMD_FP_256:PACKED_DOUBLE "
            "SIMD_COMP_INST_RETIRED:SCALAR_DOUBLE PAPI_FAD_INS PAPI_FP_INS "
            "SIMD_COMP_INST_RETIRED:SCALAR_SINGLE PAPI_FML_INS "
            "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE "
            "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE "
            "FP_COMP_OPS_EXE:SSE_PACKED_SINGLE "
            "FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE "
            "SIMD_FP_256:PACKED_SINGLE:PACKED_DOUBLE "
            "SSEX_UOPS_RETIRED:SCALAR_DOUBLE:SCALAR_SINGLE "
            "SIMD_COMP_INST_RETIRED:SCALAR_SINGLE:SCALAR_DOUBLE "
            "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:"
            "SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87" },
        {5000000,  0, "PAPI_L2_DCA PAPI_L2_TCA PAPI_L2_ICA PAPI_L2_DCM "
            "PAPI_L2_ICM PAPI_L2_TCM PAPI_L3_TCM PAPI_L3_TCA PAPI_BR_INS "
            "L2_RQSTS:DEMAND_DATA_RD_HIT PAPI_BR_MSP "},
        {500000,   0, "PAPI_TLB_DM DTLB_LOAD_MISSES:WALK_DURATION PAPI_TLB_IM "
            "ITLB_MISSES:WALK_DURATION DTLB_LOAD_MISSES:CAUSES_A_WALK "
            "DTLB_STORE_MISSES:CAUSES_A_WALK"},
        {100000,   0, "" },
        {0,        0, "" }
    };

    if (PAPI_OK != PAPI_event_name_to_code((char *)name, &event_code)) {
        OUTPUT(("%s [%s]", _ERROR("event not available (name->code)"), name));
        return PERFEXPERT_FAILURE;
    }
    if (PAPI_OK != PAPI_query_event(event_code)) {
        OUTPUT(("%s [%s]", _ERROR("event not available (query code)"), name));
        return PERFEXPERT_FAILURE;
    }
    if (PAPI_OK != PAPI_get_event_info(event_code, &info)) {
        OUTPUT(("%s [%s]", _ERROR("event info not available"), name));
        return PERFEXPERT_FAILURE;
    }

    if ((0 == strlen(info.derived)) ||
        (0 == strcmp(info.derived, "NOT_DERIVED")) ||
        (0 == strcmp(info.derived, "DERIVED_CMPD"))) {

        while (0 != strcmp("", sample[cat].events)) {
            if (NULL != strstr(sample[cat].events, name)) {
                break;
            }
            cat++;
        }

        if (0 >= (rate = get_prime(sample[cat].last ?
            sample[cat].last : sample[cat + 1].end, sample[cat].end))) {
            OUTPUT(("%s [%s]", _ERROR("unable to get prime"), name));
            return PERFEXPERT_FAILURE;
        }
        sample[cat].last = rate;
        return rate;
    }
    return PERFEXPERT_FAILURE;
}

#ifdef __cplusplus
}
#endif

// EOF