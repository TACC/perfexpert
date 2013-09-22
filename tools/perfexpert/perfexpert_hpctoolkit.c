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
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists_and_is_exec(
        globals.program)) {
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
    char   temp_str[2][BUFFER_SIZE];
    char   *argv[5];
    test_t test;

    /* Arguments to run hpcstruct */
    argv[0] = HPCSTRUCT;
    argv[1] = "--output";
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[2] = temp_str[0];
    argv[3] = globals.program_full;
    argv[4] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 3; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%s/%s.output", globals.stepdir, HPCSTRUCT);
    test.output = temp_str[1];
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    return fork_and_wait(&test, argv);
}

/* run_hpcrun */
int run_hpcrun(void) {
    FILE   *exp_file_FP;
    char   *exp_file;
    int    input_line = 0;
    char   buffer[BUFFER_SIZE];
    char   *temp_str;
    experiment_t *experiment;
    perfexpert_list_t experiments;
    int    rc = PERFEXPERT_SUCCESS;
    char   *argv[2];
    test_t test;
    struct timespec time_start, time_end, time_diff;

    /* Open experiment file (it is a list of arguments which I use to run) */
    exp_file = (char *)malloc(strlen(PERFEXPERT_ETCDIR) +
        strlen(EXPERIMENT_FILE) + 2);
    
    if (NULL == exp_file) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(exp_file, strlen(PERFEXPERT_ETCDIR) + strlen(EXPERIMENT_FILE) + 2);
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR, EXPERIMENT_FILE);

    if (NULL == (exp_file_FP = fopen(exp_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), exp_file));
        free(exp_file);
        return PERFEXPERT_ERROR;
    }

    /* Initiate the list of experiments */
    perfexpert_list_construct((perfexpert_list_t *)&experiments);

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, exp_file_FP)) {
        int temp;

        /* Ignore comments */
        input_line++;
        if (0 == strncmp("#", buffer, 1)) {
            continue;
        }

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* Create a list item for this code experiment */
            experiment = (experiment_t *)malloc(sizeof(experiment_t));
            if (NULL == experiment) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }

            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)experiment);
            perfexpert_list_append(&experiments,
                                   (perfexpert_list_item_t *)experiment);
            experiment->argc = 0;

            /* Add PREFIX to argv */
            if (NULL != globals.prefix) {
                temp_str = (char *)malloc(strlen(globals.prefix) + 1);
                if (NULL == temp_str) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    rc = PERFEXPERT_ERROR;
                    goto CLEANUP;
                }
                bzero(temp_str, strlen(globals.prefix) + 1);
                sprintf(temp_str, "%s", globals.prefix);

                experiment->argv[experiment->argc] = strtok(temp_str, " ");
                do {
                    experiment->argc++;
                } while (experiment->argv[experiment->argc] = strtok(NULL,
                    " "));
            }

            /* Arguments to run hpcrun */
            experiment->argv[experiment->argc] = HPCRUN;
            experiment->argc++;
            experiment->argv[experiment->argc] = "--output";
            experiment->argc++;
            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/measurements", globals.stepdir);
            experiment->argv[experiment->argc] = temp_str;
            experiment->argc++;

            /* Move to next line */
            continue;
        }

        /* Remove the \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Read event */
        experiment->argv[experiment->argc] = "--event";
        experiment->argc++;

        temp_str = (char *)malloc(BUFFER_SIZE);
        if (NULL == temp_str) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            rc = PERFEXPERT_ERROR;
            goto CLEANUP;
        }
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s", buffer);
        experiment->argv[experiment->argc] = temp_str;
        experiment->argc++;

        /* Move to next line */
        continue;
    }

    /* Close file and free memory */
    fclose(exp_file_FP);
    free(exp_file);

    /* For each experiment... */
    input_line = 1;
    OUTPUT_VERBOSE((5, "   %s", _YELLOW("running experiments")));
    experiment = (experiment_t *)perfexpert_list_get_first(&experiments);
    while ((perfexpert_list_item_t *)experiment != &(experiments.sentinel)) {
        int count = 0;

        /* Run the BEFORE program */
        if (NULL != globals.before) {
            argv[0] = globals.before;
            argv[1]; NULL;

            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/before.%d.output", globals.stepdir,
                    input_line);
            experiment->test.output = temp_str;
            test.output = temp_str;
            test.input  = NULL;
            test.info   = globals.before;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _BOLDRED("error running"),
                        globals.before));
            }
            free(temp_str);
        }

        /* Ok, now we have to add the program and... */
        experiment->argv[experiment->argc] = globals.program_full;
        experiment->argc++;

        /* ...and the program arguments to experiment's argv */
        while ((globals.prog_arg_pos + count) < globals.main_argc) {
            experiment->argv[experiment->argc] =
                globals.main_argv[globals.prog_arg_pos + count];
            experiment->argc++;
            count++;
        }
        experiment->argv[experiment->argc] = NULL;

        /* The super-ninja test sctructure */
        temp_str = (char *)malloc(BUFFER_SIZE);
        if (NULL == temp_str) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            rc = PERFEXPERT_ERROR;
            goto CLEANUP;
        }
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s/hpcrun.%d.output", globals.stepdir, input_line);
        experiment->test.output = temp_str;
        experiment->test.input  = NULL;
        experiment->test.info   = globals.program;

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
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;

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
        OUTPUT(("   [%d] run in %lld.%.9ld seconds", input_line,
            (long long)time_diff.tv_sec, time_diff.tv_nsec));

        /* Run the AFTER program */
        if (NULL != globals.after) {
            argv[0] = globals.after;
            argv[1]; NULL;

            temp_str = (char *)malloc(BUFFER_SIZE);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                rc = PERFEXPERT_ERROR;
                goto CLEANUP;
            }
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str, "%s/after.%d.output", globals.stepdir,
                    input_line);
            experiment->test.output = temp_str;
            test.output = temp_str;
            test.input  = NULL;
            test.info   = globals.after;

            if (0 != fork_and_wait(&test, (char **)argv)) {
                OUTPUT(("   %s [%s]", _BOLDRED("error running"),
                        globals.after));
            }
            free(temp_str);
        }

        /* Move to the next experiment */
        input_line++;
        experiment = (experiment_t *)perfexpert_list_get_next(experiment);
    }

    CLEANUP:
    /* Free memory */
    // TODO: free list of experiments, and for each experiment argv and test

    return rc;
}

/* run_hpcrun_knc */
int run_hpcrun_knc(void) {
    const char blank[] = " \t\r\n";
    FILE *script_FP;
    FILE *experiment_FP;
    char file[BUFFER_SIZE];
    char script[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char experiment[BUFFER_SIZE];
    int  experiment_count = 0;
    char *argv[4];
    test_t test;
    int  count = 0;
    
    /* Open experiment file (it is a list of arguments which I use to run) */
    bzero(file, BUFFER_SIZE);
    sprintf(file, "%s/%s", PERFEXPERT_ETCDIR, MIC_EXPERIMENT_FILE);
    if (NULL == (experiment_FP = fopen(file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), file));
        return PERFEXPERT_ERROR;
    }

    /* If this command should run on the MIC, encapsulate it in a script */
    bzero(script, BUFFER_SIZE);
    sprintf(script, "%s/knc_hpcrun.sh", globals.stepdir);

    if (NULL == (script_FP = fopen(script, "w"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open file"), script));
        return PERFEXPERT_ERROR;
    }
    fprintf(script_FP, "#!/bin/sh");

    /* Fill the script file with all the experiments, before, and after */
    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, experiment_FP)) {
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, blank) == strlen(buffer))) {
            continue;
        }

        /* Is this line a new experiment? */
        if (0 == strncmp("%", buffer, 1)) {
            /* In case this is not the first experiment...*/
            if (0 < experiment_count) {
                /* Add the program and the program arguments to experiment */
                fprintf(script_FP, " %s", globals.program_full);
                count = 0;
                while ((globals.prog_arg_pos + count) < globals.main_argc) {
                    fprintf(script_FP, " %s",
                        globals.main_argv[globals.prog_arg_pos + count]);
                    count++;
                }

                /* Add the AFTER program */
                if (NULL != globals.knc_after) {
                    fprintf(script_FP, "\n\n# Run the AFTER command\n");
                    fprintf(script_FP, "%s", globals.knc_after);
                }
            }

            /* Add the BEFORE program */
            if (NULL != globals.knc_before) {
                fprintf(script_FP, "\n\n# Run the BEFORE command\n");
                fprintf(script_FP, "%s", globals.knc_before);
            }

            fprintf(script_FP, "\n\n# Run HPCRUN (%d)\n", experiment_count);
            /* Add PREFIX */
            if (NULL != globals.knc_prefix) {
                fprintf(script_FP, "%s ", globals.knc_prefix);
            }

            /* Arguments to run hpcrun */
            fprintf(script_FP, "%s --output %s/measurements", HPCRUN,
                globals.stepdir);

            /* Move to next line of the experiment file */
            experiment_count++;
            continue;
        }

        /* Remove the \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Add the event */
        fprintf(script_FP, " --event %s", buffer);

        /* Move to next line */
        continue;
    }

    /* Add the program and the program arguments to experiment's */
    count = 0;
    fprintf(script_FP, " %s", globals.program_full);
    while ((globals.prog_arg_pos + count) < globals.main_argc) {
        fprintf(script_FP, " %s", globals.main_argv[globals.prog_arg_pos +
            count]);
        count++;
    }

    /* Add the AFTER program */
    if (NULL != globals.knc_after) {
        fprintf(script_FP, "\n\n# Run the AFTER command\n");
        fprintf(script_FP, "%s", globals.knc_after);
    }
    fprintf(script_FP, "\n\nexit 0\n\n# EOF\n");

    /* Close files and set mode */
    fclose(experiment_FP);
    fclose(script_FP);
    if (-1 == chmod(script, S_IRWXU)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: unable to set script mode"),
            script));
        return PERFEXPERT_ERROR;
    }

    /* The super-ninja test sctructure */
    bzero(file, BUFFER_SIZE);
    sprintf(file, "%s/knc_hpcrun.output", globals.stepdir);

    test.output = file;
    test.input  = NULL;
    test.info   = globals.program;
    
    argv[0] = "ssh";
    argv[1] = globals.knc;
    argv[2] = script;
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
            return PERFEXPERT_ERROR;

        case PERFEXPERT_FAILURE:
        case 255:
            OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
            return PERFEXPERT_FAILURE;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "   [ %s  ]", _BOLDGREEN("OK")));
            return PERFEXPERT_SUCCESS;

        default:
            OUTPUT_VERBOSE((7, "   [UNKNO]"));
            return PERFEXPERT_SUCCESS;
    }
}

/* run_hpcprof */
int run_hpcprof(void) {
    char   temp_str[4][BUFFER_SIZE];
    char   *argv[9];
    test_t test;

    /* Arguments to run hpcprof */
    argv[0] = HPCPROF;
    argv[1] = "--force-metric";
    argv[2] = "--metric=thread";
    argv[3] = "--struct";
    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s.hpcstruct", globals.stepdir, globals.program);
    argv[4] = temp_str[0];
    argv[5] = "--output";
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%s/database", globals.stepdir);
    argv[6] = temp_str[1];
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/measurements", globals.stepdir);
    argv[7] = temp_str[2];
    argv[8] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i <= 7; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s/%s.output", globals.stepdir, HPCPROF);
    test.output = temp_str[3];
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait_and_pray */
    return fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
