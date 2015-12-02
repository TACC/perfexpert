/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
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

// This function takes the output of running VTune and creates a report CSV file
// The results of running VTune are in results_folder
// The output of this function is in parse_file
// The command it executes is like:
    // amplxe-cl -R hw-events -r results_folder  -group-by=function -format=csv -report-output=parse_file -csv-delimiter=','
    //  amplxe-cl -report hw-events -group-by=function -group-by=module -group-by=source-file -group-by=source-line -group-by=thread -r results_folder -format=csv -report-output=parse-file -csv-delimiter=','
int create_report(char* results_folder, const char* parse_file) {
    struct timespec time_start, time_end, time_diff;
    char *argv[MAX_ARGUMENTS_COUNT];
    int argc = 0, i, rc;
    char buffer[MAX_BUFFER_SIZE];
    test_t test;

    argv[argc] = VTUNE_CL_BIN;
    argc++;
    argv[argc] = VTUNE_ACT_REPORT;
    argc++;
    argv[argc] = VTUNE_RPT_HW;
    argc++;
    argv[argc] = "-group-by=function";
    argc++;
    argv[argc] = "-group-by=module";
    argc++;
    argv[argc] = "-group-by=source-file";
    argc++;
    argv[argc] = "-group-by=source-line";
    argc++;
    argv[argc] = "-group-by=thread";
    argc++;
    argv[argc] = "-r";
    argc++;
    argv[argc] = my_module_globals.res_folder;
    argc++;
    argv[argc] = "-format=csv";
    argc++;
    strcpy(buffer, "-report-output=");
    strcat(buffer, parse_file);
    argv[argc] = buffer;
    argc++;
    argv[argc] = "-csv-delimiter=','";
    argc++;

    argv[argc] = NULL;

    /* Not using OUTPUT_VERBOSEbecause I want only one line */
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

int parse_line(char* line, char *argv[], int *argc) {
    char* tok;
    (*argc)=0;
    tok = strtok(line, ",");
    while (tok != NULL) {
        argv[(*argc)] = tok;
        /* Remove possible trailing single quotes from the text */
        if (argv[(*argc)][0] == '\'') {
            argv[(*argc)]++;
        }
        if (argv[(*argc)][strlen(argv[(*argc)])-1] == '\'') {
            argv[(*argc)][strlen(argv[(*argc)])-1] = 0;
        }
        (*argc)++;
        tok = strtok(NULL, ",");
    }
    return PERFEXPERT_SUCCESS;
}

/* Receives a string like 'OMP Worker Thread #4 (TID: 42223)'
 * and returns the thread number (right after #)
 */
int get_thread_number(const char *argv) {
    OUTPUT_VERBOSE((6, "checking thread %s", argv));
    if (strpbrk(argv, "TID") != NULL) {
        OUTPUT_VERBOSE((6, "INVALID thread, returning 0"));
        return 0;
    }

    const char *p1 = strstr(argv, "#")+1;
    const char *p2 = strstr(p1, " (");
    int len = p2-p1;
    int val;
    char *res;
    PERFEXPERT_ALLOC(char, res, len+1);
    strncpy(res, p1, len);
    res[len]='\0';
    val = atoi(res);
    PERFEXPERT_DEALLOC(res);
    return val;
}

int parse_hotspot_name(char *hp, char *name) {
    char * tok;
    tok = strtok(hp, "@$");
    if (tok == NULL) {
        name = hp;
    }
    else {
        name = tok;
    }
    return PERFEXPERT_SUCCESS;
}

/*
This function processes a CSV file that looks like this:
  Function,Module,Source File,Source Line,Thread,Hardware Event Count:ARITH.FPU_DIV...
  bpnn_adjust_weights$omp$parallel_for@316,backprop,backprop.c,324,OMP Worker Thread #1 (TID: 42220),0,0,0,4000006,0,0
*/

int parse_report(const char * parse_file, vtune_hw_profile_t *profile) {
    FILE * in = fopen(parse_file, "r");
    if (in == NULL) {
        OUTPUT(("%s", _ERROR("unable to open the report generated by VTune")));
        return PERFEXPERT_ERROR;
    }
    char line[MAX_BUFFER_SIZE];
    char header[MAX_BUFFER_SIZE];
    int i;
    char *events[MAX_ARGUMENTS_COUNT], *c_events[MAX_ARGUMENTS_COUNT];
    int argc;

    OUTPUT_VERBOSE((8, "%s", "processing VTune's results"));
    fgets(header, MAX_BUFFER_SIZE, in);
    /* Parse the header here */
    if (PERFEXPERT_SUCCESS != parse_line(header, c_events, &argc)) {
        OUTPUT(("%s", _ERROR("unable to parse the report generated by VTune")));
        fclose(in);
        return PERFEXPERT_ERROR;
    }

    /* The two first columns are not event names, so skip them
       Also, VTune adds extra text before and after the counter, so
       we have to process that. We remove everything before the first ':'
       and the final ':Self'
    */
    for (i = 5; i < argc; ++i)  {
        char * tok;
        int parts = 0;
        tok = strtok(c_events[i], ":");
        while (tok != NULL) {
            if (parts == 1) {
                events[i] = tok;
                break;
            }
            tok = strtok(NULL, ",");
            parts++;
        }
        if (parts == 0) {
            events[i] = c_events[i];
        }
        tok = strstr(events[i], ":Self");
        if (tok != NULL) {
            sprintf(events[i]+(strlen(events[i])-5), "\0");
        }
        /* Replace the '.' on the metric, GNU libmatheval does not like them... */
        perfexpert_string_replace_char(events[i], '.', '_');
    }

    /* Read the rest of the csv file */
    while (fgets(line,MAX_BUFFER_SIZE, in)) {
        char *argv[MAX_ARGUMENTS_COUNT];
        vtune_hotspots_t *hotspot;

        OUTPUT_VERBOSE((9, "processing line %s", line));
        if (PERFEXPERT_SUCCESS != parse_line(line, argv, &argc)) {
            OUTPUT(("%s", _ERROR("unable to parse the report generated by VTune")));
            fclose(in);
            return PERFEXPERT_ERROR;
        }

        PERFEXPERT_ALLOC(vtune_hotspots_t, hotspot, sizeof(vtune_hotspots_t));
        PERFEXPERT_ALLOC(char, hotspot->name, strlen(argv[0]));
        PERFEXPERT_ALLOC(char, hotspot->module, strlen(argv[1]));
        PERFEXPERT_ALLOC(char, hotspot->src_file, strlen(argv[2]));
        parse_hotspot_name(argv[0], argv[0]);
        strcpy(hotspot->name, argv[0]);
        strcpy(hotspot->module, argv[1]);
        strcpy(hotspot->src_file, argv[2]);
        hotspot->src_line = atoi(argv[3]);
        strcpy(hotspot->name_md5, perfexpert_md5_string(hotspot->name));

        hotspot->thread = get_thread_number(argv[4]);

        /* TODO(agomez) */
        hotspot->mpi_rank = 0;

        perfexpert_list_item_construct((perfexpert_list_item_t *)hotspot);
        perfexpert_list_construct(&hotspot->events);
        /* Process the list of arguments */
        /* TODO(agomez): only collect statistics about a specific set of functions??
         */
        for (i = 5; i < argc; ++i) {
            vtune_event_t *e;
            PERFEXPERT_ALLOC(vtune_event_t, e, sizeof(vtune_event_t));
            PERFEXPERT_ALLOC(char, e->name, strlen(events[i])+1);
            strcpy(e->name, events[i]);
            strcpy(e->name_md5, perfexpert_md5_string(e->name));
            e->samples = 0;
            e->value = atol(argv[i]);
            perfexpert_hash_add_str(hotspot->events_by_name, name_md5, e);
            perfexpert_list_append(&(hotspot->events), (perfexpert_list_item_t *) e);
        }
        perfexpert_hash_add_str(profile->hotspots_by_name, name_md5, hotspot);
        perfexpert_list_append(&(profile->hotspots), (perfexpert_list_item_t *) hotspot);
    }
    fclose(in);
    OUTPUT_VERBOSE((6, "Output file parsed"));
    return PERFEXPERT_SUCCESS;
}


int run_amplxe_cl_mic(void) {
    /* TODO(agomez) */
    return PERFEXPERT_SUCCESS;
}

/* run_amplxe_cl */
int run_amplxe_cl(void) {
    struct timespec time_start, time_end, time_diff;
    vtune_event_t *event = NULL, *t = NULL;
    char events[MAX_BUFFER_SIZE];
    char *argv[MAX_ARGUMENTS_COUNT];
    int argc = 0, i = 0, rc;
    test_t test;
    static char template[] = "vtune-XXXXXX";

    sprintf(my_module_globals.res_folder, "%s/%s", globals.moduledir, template);
    mktemp(my_module_globals.res_folder);

    /* Set the events we want to cVTune collect */
    bzero(events, MAX_BUFFER_SIZE);
    perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
        strcat(events, event->name);
        strcat(events, ",");
    }
    events[strlen(events) - 1] = '\0';

    OUTPUT_VERBOSE((8, "   %s %s", _YELLOW("events:"), events));

    /* Run the BEFORE program */
    if (NULL != my_module_globals.before[0]) {
        OUTPUT_VERBOSE((8, " %s", "running the before program"));
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
    argv[argc] = VTUNE_ACT_COLLECTWITH;
    argc++;
    argv[argc] = "runsa";
    argc++;
    argv[argc] = "-r";
    argc++;
    argv[argc] = my_module_globals.res_folder;
    argc++;
    /* argv[argc] = "runsa-knc";
       argc++;
    */
    argv[argc] = "-knob";
    argc++;
    char * events_opt = (char *) malloc ((strlen(events)+14)*sizeof(char));
    strcpy(events_opt, "event-config=");
    strcat(events_opt, events);
    argv[argc]=events_opt;
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
    /* Not using OUTPUT_VERBOSEbecause I want only one line */
    if (8 <= globals.verbose) {
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* The super-ninja test sctructure */
    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
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

int collect_results(vtune_hw_profile_t *profile) {
    int fid = -1;
    static char template[] = "vtune-report-XXXXXX";
    char csv_name[MAX_FILENAME];

    sprintf(csv_name, "%s/%s", my_module_globals.res_folder, template);
    mktemp(csv_name);
    OUTPUT_VERBOSE((9, "Creating report at %s", csv_name));
    if (PERFEXPERT_SUCCESS != create_report("", csv_name)) {
        OUTPUT(("%s", _ERROR("unable to create a report file from the \
                              results generated by VTune")));
        PERFEXPERT_DEALLOC(profile);
        return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE((9, "Parsing report %s", csv_name));
    if (PERFEXPERT_SUCCESS != parse_report(csv_name, profile)) {
         OUTPUT(("%s", _ERROR("unable to collect the results generated by VTune")));
         PERFEXPERT_DEALLOC(profile);
         return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE((8, "%s", "VTune's results collected"));
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
