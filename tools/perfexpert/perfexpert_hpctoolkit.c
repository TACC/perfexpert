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
#include "perfexpert_hpctoolkit.h"
#include "perfexpert_output.h"
#include "perfexpert_time.h"
#include "perfexpert_util.h"
#include "install_dirs.h"

/* measurements_hpctoolkit */
int measurements_hpctoolkit(void) {
    /* First of all, does the file exist? (it is just a double check) */
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_exec(globals.program)) {
        return PERFEXPERT_ERROR;
    }
    /* Create the program structure file */
    if (PERFEXPERT_SUCCESS != run_hpcstruct()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcstruct")));
        return PERFEXPERT_ERROR;
    }
    /* Collect measurements */
    if (NULL == globals.knc) {
        if (PERFEXPERT_SUCCESS != run_hpcrun()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun")));
            return PERFEXPERT_ERROR;
        }
    } else {
        if (PERFEXPERT_SUCCESS != run_hpcrun_knc()) {
            OUTPUT(("%s", _ERROR("Error: unable to run hpcrun on KNC")));
            OUTPUT(("Are you adding the flags to compile for MIC?"));                    
            return PERFEXPERT_ERROR;
        }
    }
    /* Sumarize results */
    if (PERFEXPERT_SUCCESS != run_hpcprof()) {
        OUTPUT(("%s", _ERROR("Error: unable to run hpcprof")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* run_hpcstruct */
int run_hpcstruct(void) {
    char *argv[5];
    int rc = PERFEXPERT_SUCCESS;
    test_t test;

    /* Arguments to run hpcstruct */
    argv[0] = HPCSTRUCT;
    argv[1] = "--output";
    PERFEXPERT_ALLOC(char, argv[2],
        (strlen(globals.stepdir) + strlen(globals.program) + 12));
    sprintf(argv[2], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[3] = globals.program_full;
    argv[4] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 4; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output,
        (strlen(globals.stepdir) + strlen(HPCSTRUCT) + 9));
    sprintf(test.output, "%s/%s.output", globals.stepdir, HPCSTRUCT);
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
    struct timespec time_start, time_end, time_diff;
    FILE *exp_file_FP;
    char *exp_file;
    char *exp_prefix;
    int count;
    char buffer[BUFFER_SIZE];
    char *argv[2];
    int input_line = 0;
    int rc = PERFEXPERT_SUCCESS;
    test_t test;
    experiment_t *experiment;
    perfexpert_list_t experiments;

    /* Open experiment file (it is a list of arguments which I use to run) */
    PERFEXPERT_ALLOC(char, exp_file,
        (strlen(PERFEXPERT_ETCDIR) + strlen(HPCTOOLKIT_EXPERIMENT_FILE) + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR, HPCTOOLKIT_EXPERIMENT_FILE);

    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), exp_file));
        PERFEXPERT_DEALLOC(exp_file);
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(exp_file);

    /* Initiate the list of experiments */
    perfexpert_list_construct(&experiments);

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, exp_file_FP)) {
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* Create a list item for this code experiment */
            PERFEXPERT_ALLOC(experiment_t, experiment, sizeof(experiment_t));
            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)experiment);
            perfexpert_list_append(&experiments,
                (perfexpert_list_item_t *)experiment);
            experiment->argc = 0;

            /* Add PREFIX to argv */
            if (NULL != globals.prefix) {
                PERFEXPERT_ALLOC(char, exp_prefix,
                    (strlen(globals.prefix) + 1));
                sprintf(exp_prefix, "%s", globals.prefix);

                experiment->argv[0] = strtok(exp_prefix, " ");
                experiment->argc = 1;
                while (experiment->argv[experiment->argc] = strtok(NULL, " ")) {
                    experiment->argc++;
                }
            }

            /* Arguments to run hpcrun */
            experiment->argv[experiment->argc] = HPCRUN;
            experiment->argc++;
            experiment->argv[experiment->argc] = "--output";
            experiment->argc++;
            PERFEXPERT_ALLOC(char, experiment->argv[experiment->argc],
                (strlen(globals.stepdir) + 14));
            sprintf(experiment->argv[experiment->argc], "%s/measurements",
                globals.stepdir);
            experiment->argc++;

            /* Move to next line */
            continue;
        }

        /* Read event */
        experiment->argv[experiment->argc] = "--event";
        experiment->argc++;
        PERFEXPERT_ALLOC(char, experiment->argv[experiment->argc],
            (strlen(buffer) + 1));
        sprintf(experiment->argv[experiment->argc], "%s", buffer);
        experiment->argc++;
    }

    /* Close file */
    fclose(exp_file_FP);

    /* For each experiment... */
    OUTPUT_VERBOSE((5, "   %s", _YELLOW("running experiments")));
    while (0 < perfexpert_list_get_size(&experiments)) {
        int count = 0;

        experiment = (experiment_t *)perfexpert_list_get_first(&experiments);
        input_line++;

        /* Run the BEFORE program */
        if (NULL != globals.before) {
            argv[0] = globals.before;
            argv[1]; NULL;

            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/before.%d.output", globals.stepdir, count);
            test.input = NULL;
            test.info = globals.before;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _RED("error running"), globals.before));
            }
            PERFEXPERT_DEALLOC(test.output);
        }

        /* Ok, now we have to add the program and... */
        experiment->argv[experiment->argc] = globals.program_full;
        experiment->argc++;

        /* ...and the program arguments to experiment's argv */
        while (NULL != globals.program_argv[count]) {
            experiment->argv[experiment->argc] = globals.program_argv[count];
            experiment->argc++;
            count++;
        }
        experiment->argv[experiment->argc] = NULL;

        /* The super-ninja test sctructure */
        PERFEXPERT_ALLOC(char, experiment->test.output,
            (strlen(globals.stepdir) + 20));
        sprintf(experiment->test.output, "%s/hpcrun.%d.output", globals.stepdir,
            input_line);
        experiment->test.input = NULL;
        experiment->test.info = globals.program;

        /* Not using OUTPUT_VERBOSE because I want only one line */
        if (8 <= globals.verbose) {
            int i;
            printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
            for (i = 0; i < experiment->argc; i++) {
                printf(" %s", experiment->argv[i]);
            }
            printf("\n");
        }

        /* Run program and test return code (should I really test it?) */
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        switch (fork_and_wait(&(experiment->test), (char **)experiment->argv)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
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
        OUTPUT(("   [%d] %lld.%.9ld seconds (includes measurement overhead)",
            input_line, (long long)time_diff.tv_sec, time_diff.tv_nsec));

        /* Run the AFTER program */
        if (NULL != globals.after) {
            argv[0] = globals.after;
            argv[1]; NULL;

            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/after.%d.output", globals.stepdir, count);
            test.input = NULL;
            test.info = globals.after;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _RED("error running"), globals.after));
            }
            PERFEXPERT_DEALLOC(test.output);
        }

        /* Remove experiment from list */
        perfexpert_list_remove_item(&experiments,
            (perfexpert_list_item_t *)(experiment));
        PERFEXPERT_DEALLOC(experiment->test.output);
        PERFEXPERT_DEALLOC(experiment);
        experiment = (experiment_t *)perfexpert_list_get_first(&experiments);
    }
    return rc;
}

/* run_hpcrun_knc */
int run_hpcrun_knc(void) {
    const char blank[] = " \t\r\n";
    FILE *script_file_FP;
    char *script_file;
    FILE *exp_file_FP;
    char *exp_file;
    char buffer[BUFFER_SIZE];
    char experiment[BUFFER_SIZE];
    int  experiment_count = 0;
    char *argv[4];
    test_t test;
    int  count = 0;

    /* Open experiment file (it is a list of arguments which I use to run) */
    PERFEXPERT_ALLOC(char, exp_file,
        (strlen(PERFEXPERT_ETCDIR) + strlen(HPCTOOLKIT_MIC_EXPERIMENT_FILE)
            + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR,
        HPCTOOLKIT_MIC_EXPERIMENT_FILE);

    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), exp_file));
        PERFEXPERT_DEALLOC(exp_file);
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(exp_file);

    /* If this command should run on the MIC, encapsulate it in a script */
    PERFEXPERT_ALLOC(char, script_file, (strlen(globals.stepdir) + 15));
    sprintf(script_file, "%s/knc_hpcrun.sh", globals.stepdir);

    if (NULL == (script_file_FP = fopen(script_file, "w"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), script_file));
        return PERFEXPERT_ERROR;
    }

    fprintf(script_file_FP, "#!/bin/sh");
    /* Fill the script file with all the experiments, before, and after */
    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, exp_file_FP)) {
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* In case this is not the first experiment...*/
            if (0 < experiment_count) {
                /* Add the program and the program arguments to experiment */
                fprintf(script_file_FP, " %s", globals.program_full);
                count = 0;
                while (NULL != globals.program_argv[count]) {
                    fprintf(script_file_FP, " %s", globals.program_argv[count]);
                    count++;
                }

                /* Add the AFTER program */
                if (NULL != globals.knc_after) {
                    fprintf(script_file_FP, "\n\n# AFTER command\n");
                    fprintf(script_file_FP, "%s", globals.knc_after);
                }
            }

            /* Add the BEFORE program */
            if (NULL != globals.knc_before) {
                fprintf(script_file_FP, "\n\n# BEFORE command\n");
                fprintf(script_file_FP, "%s", globals.knc_before);
            }

            fprintf(script_file_FP, "\n\n# HPCRUN (%d)\n", experiment_count);

            /* Add PREFIX */
            if (NULL != globals.knc_prefix) {
                fprintf(script_file_FP, "%s ", globals.knc_prefix);
            }

            /* Arguments to run hpcrun */
            fprintf(script_file_FP, "%s --output %s/measurements", HPCRUN,
                globals.stepdir);

            /* Move to next line of the experiment file */
            experiment_count++;
            continue;
        }

        /* Add the event */
        fprintf(script_file_FP, " --event %s", buffer);
    }

    /* Add the program and the program arguments to experiment's */
    // count = 0;
    // fprintf(script_file_FP, " %s", globals.program_full);
    // while ((globals.prog_arg_pos + count) < globals.main_argc) {
    //     fprintf(script_file_FP, " %s", globals.main_argv[globals.prog_arg_pos +
    //         count]);
    //     count++;
    // }

    /* Add the AFTER program */
    if (NULL != globals.knc_after) {
        fprintf(script_file_FP, "\n\n# AFTER command\n");
        fprintf(script_file_FP, "%s", globals.knc_after);
    }
    fprintf(script_file_FP, "\n\nexit 0\n\n# EOF\n");

    /* Close files and set mode */
    fclose(exp_file_FP);
    fclose(script_file_FP);
    if (-1 == chmod(script_file, S_IRWXU)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: unable to set script mode"),
            script_file));
        PERFEXPERT_DEALLOC(script_file);
        return PERFEXPERT_ERROR;
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 19));
    sprintf(test.output, "%s/knc_hpcrun.output", globals.stepdir);
    test.input = NULL;
    test.info = globals.program;
    
    argv[0] = "ssh";
    argv[1] = globals.knc;
    argv[2] = script_file;
    argv[3] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        printf("%s    %s %s %s %s\n", PROGRAM_PREFIX, _YELLOW("command line:"),
            argv[0], argv[1], argv[2]);
    }

    /* Run program and test return code (should I really test it?) */
    switch (fork_and_wait(&test, argv)) {
        case PERFEXPERT_ERROR:
            OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
            PERFEXPERT_DEALLOC(script_file);
            PERFEXPERT_DEALLOC(test.output);
            return PERFEXPERT_ERROR;

        case PERFEXPERT_FAILURE:
        case 255:
            OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
            PERFEXPERT_DEALLOC(script_file);
            PERFEXPERT_DEALLOC(test.output);
            return PERFEXPERT_FAILURE;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "   [ %s  ]", _BOLDGREEN("OK")));
            PERFEXPERT_DEALLOC(script_file);
            PERFEXPERT_DEALLOC(test.output);
            return PERFEXPERT_SUCCESS;

        default:
            OUTPUT_VERBOSE((7, "   [UNKNO]"));
            PERFEXPERT_DEALLOC(script_file);
            PERFEXPERT_DEALLOC(test.output);
            return PERFEXPERT_SUCCESS;
    }
}

/* run_hpcprof */
int run_hpcprof(void) {
    int rc = PERFEXPERT_SUCCESS;
    char *argv[9];
    test_t test;

    /* Arguments to run hpcprof */
    argv[0] = HPCPROF;
    argv[1] = "--force-metric";
    argv[2] = "--metric=thread";
    argv[3] = "--struct";
    PERFEXPERT_ALLOC(char, argv[4],
        (strlen(globals.stepdir) + strlen(globals.program) + 12));
    sprintf(argv[4], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[5] = "--output";
    PERFEXPERT_ALLOC(char, argv[6], (strlen(globals.stepdir) + 10));
    sprintf(argv[6], "%s/database", globals.stepdir);
    PERFEXPERT_ALLOC(char, argv[7], (strlen(globals.stepdir) + 14));
    sprintf(argv[7], "%s/measurements", globals.stepdir);
    argv[8] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 8; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output,
        (strlen(globals.stepdir) + strlen(HPCPROF) + 9));
    sprintf(test.output, "%s/%s.output", globals.stepdir, HPCPROF);
    test.input = NULL;
    test.info = globals.program;

    /* fork_and_wait_and_pray */
    rc = fork_and_wait(&test, argv);

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
