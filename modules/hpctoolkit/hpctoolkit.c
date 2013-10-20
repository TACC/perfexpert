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
#include <libxml/parser.h>

/* PerfExpert headers */
#include "perfexpert/perfexpert.h" // Required to capture globals
#include "analyzer/analyzer_types.h" 
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_fork.h"
#include "perfexpert_hash.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"
#include "perfexpert_time.h"
#include "perfexpert_util.h"
#include "install_dirs.h"

/* Module headers */
#include "perfexpert_module.h" 
#include "hpctoolkit.h" 

/* Global variable to define the module itself */
perfexpert_module_t myself_module;

/* module_measurements */
int perfexpert_tool_measurements(void) {
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
static int run_hpcstruct(void) {
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
static int run_hpcrun(void) {
    struct timespec time_start, time_end, time_diff;
    FILE *exp_file_FP;
    char *exp_file;
    char *exp_prefix;
    char buffer[BUFFER_SIZE];
    int input_line = 0;
    int rc = PERFEXPERT_SUCCESS;
    int i = 0;
    test_t test;
    experiment_t *experiment;
    perfexpert_list_t experiments;

    /* Open experiment file (it is a list of arguments which I use to run) */
    PERFEXPERT_ALLOC(char, exp_file, (strlen(PERFEXPERT_ETCDIR) +
        strlen(PERFEXPERT_TOOL_EXPERIMENT_FILE) + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR,
        PERFEXPERT_TOOL_EXPERIMENT_FILE);

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
            i = 0;
            while (NULL != globals.prefix[i]) {
                experiment->argv[experiment->argc] = globals.prefix[i];
                experiment->argc++;
                i++;
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
        experiment = (experiment_t *)perfexpert_list_get_first(&experiments);
        input_line++;

        /* Run the BEFORE program */
        if (NULL != globals.before[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/before.%d.output", globals.stepdir,
                input_line);
            test.input = NULL;
            test.info = globals.before[0];

            if (0 != fork_and_wait(&test, (char **)globals.before)) {
                OUTPUT(("   %s", _RED("error running 'before' command")));
            }
            PERFEXPERT_DEALLOC(test.output);
        }

        /* Ok, now we have to add the program and... */
        experiment->argv[experiment->argc] = globals.program_full;
        experiment->argc++;

        /* ...and the program arguments */
        i = 0;
        while (NULL != globals.program_argv[i]) {
            experiment->argv[experiment->argc] = globals.program_argv[i];
            experiment->argc++;
            i++;
        }

        /* The last of the Mohicans */
        experiment->argv[experiment->argc] = NULL;

        /* The super-ninja test sctructure */
        PERFEXPERT_ALLOC(char, experiment->test.output,
            (strlen(globals.stepdir) + 20));
        sprintf(experiment->test.output, "%s/hpcrun.%d.output", globals.stepdir,
            input_line);
        experiment->test.input = globals.inputfile;
        experiment->test.info = globals.program;

        /* Not using OUTPUT_VERBOSE because I want only one line */
        if (8 <= globals.verbose) {
            printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
            for (i = 0; i < experiment->argc; i++) {
                printf(" %s", experiment->argv[i]);
            }
            printf("\n");
        }

        /* (HPC)run program and test return code (should I really test it?) */
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        switch (fork_and_wait(&(experiment->test), (char **)experiment->argv)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s]", _BOLDYELLOW("ERROR")));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "   [%s ]", _BOLDRED("FAIL")));
                return PERFEXPERT_ERROR;

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
        if (NULL != globals.after[0]) {
            PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 20));
            sprintf(test.output, "%s/after.%d.output", globals.stepdir,
                input_line);
            test.input = NULL;
            test.info = globals.after[0];

            if (0 != fork_and_wait(&test, (char **)globals.after)) {
                OUTPUT(("   %s", _RED("error running 'after' command")));
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
static int run_hpcrun_knc(void) {
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
    PERFEXPERT_ALLOC(char, exp_file, (strlen(PERFEXPERT_ETCDIR) +
        strlen(PERFEXPERT_TOOL_MIC_EXPERIMENT_FILE) + 2));
    sprintf(exp_file, "%s/%s", PERFEXPERT_ETCDIR,
        PERFEXPERT_TOOL_MIC_EXPERIMENT_FILE);

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
static int run_hpcprof(void) {
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

/* module_parse_file */
int perfexpert_tool_parse_file(const char *file, perfexpert_list_t *profiles) {
    xmlDocPtr  document;
    xmlNodePtr node;
    profile_t  *profile = NULL;

    /* Open file */
    if (NULL == (document = xmlParseFile(file))) {
        OUTPUT(("%s", _ERROR("Error: malformed XML document")));
        return PERFEXPERT_ERROR;
    }

    /* Check if it is not empty */
    if (NULL == (node = xmlDocGetRootElement(document))) {
        OUTPUT(("%s", _ERROR("Error: empty XML document")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    /* Check if it is a HPCToolkit experiment file */
    if (xmlStrcmp(node->name, (const xmlChar *)"HPCToolkitExperiment")) {
        OUTPUT(("%s (%s)",
            _ERROR("Error: file is not a valid HPCToolkit experiment"), file));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((2, "   %s [HPCToolkit experiment file version %s]",
            _BLUE("Loading profiles"), xmlGetProp(node, "version")));
    }

    /* Perse the document */
    if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document, node->xmlChildrenNode,
        profiles, NULL, NULL, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to parse experiment file")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "   (%d) %s", perfexpert_list_get_size(profiles),
        _MAGENTA("code profile(s) found")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((4, "      %s", _GREEN(profile->name)));
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    xmlFreeDoc(document);

    return PERFEXPERT_SUCCESS;
}

/* hpctoolkit_parser */
static int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth) {
    file_t      *file = NULL;
    metric_t    *metric = NULL;
    module_t    *module = NULL;
    callpath_t  *callpath = NULL;
    procedure_t *procedure = NULL;

    while (NULL != node) {
        /* SecCallPathProfile */
        if (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfile")) {
            PERFEXPERT_ALLOC(profile_t, profile, sizeof(profile_t));
            profile->cycles = 0.0;
            profile->instructions = 0.0;
            profile->files_by_id = NULL;
            profile->modules_by_id = NULL;
            profile->metrics_by_id = NULL;
            profile->metrics_by_name = NULL;
            profile->procedures_by_id = NULL;
            profile->id = atoi(xmlGetProp(node, "i"));
            perfexpert_list_construct(&(profile->callees));
            perfexpert_list_construct(&(profile->hotspots));
            perfexpert_list_item_construct((perfexpert_list_item_t *)profile);

            PERFEXPERT_ALLOC(char, profile->name,
                (strlen(xmlGetProp(node, "n")) + 1));
            strcpy(profile->name, xmlGetProp(node, "n"));

            OUTPUT_VERBOSE((3, "   %s [%d] (%s)", _YELLOW("profile found:"),
                profile->id, profile->name));

            /* Add to the list of profiles */
            perfexpert_list_append(profiles, (perfexpert_list_item_t *)profile);

            /* Call the call path profile parser */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s (%s)", _ERROR("Error: unable to parse profile"),
                    xmlGetProp(node, "n")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Metric */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Metric")) {
            char *temp_str[3];

            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(metric_t, metric, sizeof(metric_t));
                metric->id = atoi(xmlGetProp(node, "i"));
                metric->value = 0.0;

                /* Save the entire metric name */
                PERFEXPERT_ALLOC(char, temp_str[0],
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(temp_str[0], xmlGetProp(node, "n"));

                temp_str[1] = strtok(temp_str[0], ".");
                if (NULL != temp_str[1]) {
                    /* Set the name (only the performance counter name) */
                    PERFEXPERT_ALLOC(char, metric->name,
                        (strlen(temp_str[1]) + 1));
                    perfexpert_string_replace_char(temp_str[1], ':', '_');
                    strcpy(metric->name, temp_str[1]);
                    strcpy(metric->name_md5,
                        perfexpert_md5_string(metric->name));

                    /* Save the thread information */
                    temp_str[1] = strtok(NULL, ".");

                    if (NULL != temp_str[1]) {
                        /* Set the experiment */
                        temp_str[2] = strtok(NULL, ".");
                        if (NULL != temp_str[2]) {
                            metric->experiment = atoi(temp_str[2]);
                        } else {
                            metric->experiment = 0;                    
                        }

                        /* Set the MPI rank */
                        temp_str[2] = strtok(temp_str[1], ",");
                        metric->mpi_rank = 0;

                        /* Set the thread ID */
                        temp_str[2] = strtok(NULL, ",");
                        if (NULL != temp_str[2]) {
                            temp_str[2][strlen(temp_str[2]) - 1] = '\0';
                            metric->thread = atoi(temp_str[2]);
                        }
                    }
                }
                PERFEXPERT_DEALLOC(temp_str[0]);
    
                OUTPUT_VERBOSE((10,
                    "   %s [%d] (%s) [rank=%d] [tid=%d] [exp=%d]",
                    _CYAN("metric"), metric->id, metric->name, metric->mpi_rank,
                    metric->thread, metric->experiment));

                /* Hash it! */
                perfexpert_hash_add_int(profile->metrics_by_id, id, metric);
            }
        }

        /* LoadModule */
        if (!xmlStrcmp(node->name, (const xmlChar *)"LoadModule")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(module_t, module, sizeof(module_t));
                module->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, module->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(module->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    module->name, &(module->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _GREEN("module"),
                    module->id, module->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->modules_by_id, id, module);
            }
        }

        /* File */
        if (!xmlStrcmp(node->name, (const xmlChar *)"File")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(file_t, file, sizeof(file_t));
                file->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, file->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(file->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    file->name, &(file->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _BLUE("file"), file->id,
                    file->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->files_by_id, id, file);
            }
        }

        /* Procedures */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Procedure")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(procedure_t, procedure, sizeof(procedure_t));
                procedure->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, procedure->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(procedure->name, xmlGetProp(node, "n"));

                procedure->type = PERFEXPERT_HOTSPOT_FUNCTION;
                procedure->instructions = 0.0;
                procedure->importance = 0.0;
                procedure->variance = 0.0;
                procedure->cycles = 0.0;
                procedure->valid = PERFEXPERT_TRUE;
                procedure->line = -1;
                procedure->file = NULL;
                procedure->module = NULL;
                procedure->lcpi_by_name = NULL;
                procedure->metrics_by_id = NULL;
                procedure->metrics_by_name = NULL;
                perfexpert_list_item_construct(
                    (perfexpert_list_item_t *)procedure);
                perfexpert_list_construct(&(procedure->metrics));

                /* Add to the profile's hash of procedures and also to the list
                 * of potential hotspots
                 */
                perfexpert_hash_add_int(profile->procedures_by_id, id,
                    procedure);
                perfexpert_list_append(&(profile->hotspots),
                    (perfexpert_list_item_t *)procedure);

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _MAGENTA("procedure"),
                    procedure->id, procedure->name));
            }
        }

        /* (Pr)ocedure and (P)rocedure(F)rame, let's make the magic happen... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"PF")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"Pr"))) {
            loopdepth = 0;
            int temp_int = 0;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "n")) || 
                (NULL == xmlGetProp(node, "f")) ||
                (NULL == xmlGetProp(node, "l")) ||
                (NULL == xmlGetProp(node, "lm")) ||
                (NULL == xmlGetProp(node, "i"))) {
                OUTPUT(("%s", _ERROR("Error: malformed callpath")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate some memory and initialize it */
            PERFEXPERT_ALLOC(callpath_t, callpath, sizeof(callpath_t));
            callpath->scope = -1;
            callpath->alien = 0;
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Find file */
            temp_int = atoi(xmlGetProp(node, "f"));
            perfexpert_hash_find_int(profile->files_by_id, &temp_int, file);
            if (NULL == file) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown file")));
                return PERFEXPERT_ERROR;
            }

            /* Find module */
            temp_int = atoi(xmlGetProp(node, "lm"));
            perfexpert_hash_find_int(profile->modules_by_id, &temp_int, module);
            if (NULL == module) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown module")));
                return PERFEXPERT_ERROR;
            }

            /* Find the procedure name */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profile->procedures_by_id, &temp_int,
                procedure);
            if (NULL == procedure) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown procedure")));
                PERFEXPERT_DEALLOC(callpath);
                return PERFEXPERT_ERROR;
            } else {
                /* Paranoia checks */
                if ((procedure->line != atoi(xmlGetProp(node, "l"))) &&
                    (-1 != procedure->line)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure line doesn't match")));
                    return PERFEXPERT_ERROR;                
                }
                if ((file->id != atoi(xmlGetProp(node, "f"))) &&
                    (NULL != procedure->file)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure file doesn't match")));
                    return PERFEXPERT_ERROR;                
                }
                if ((module->id != atoi(xmlGetProp(node, "lm"))) &&
                    (NULL != procedure->module)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure module doesn't match")));
                    return PERFEXPERT_ERROR;                
                }

                /* Set procedure file, module, and line */
                procedure->file = file;
                procedure->module = module;
                procedure->line = atoi(xmlGetProp(node, "l"));
                perfexpert_list_construct(&(procedure->metrics));
            }

            /* Set procedure and ID */
            callpath->procedure = procedure;
            callpath->id = atoi(xmlGetProp(node, "i"));

            /* Set parent */
            if (NULL == parent) {
                callpath->parent = NULL;
                perfexpert_list_append(&(profile->callees),
                    (perfexpert_list_item_t *)callpath);
            } else {
                callpath->parent = parent;
                perfexpert_list_append(&(parent->callees),
                    (perfexpert_list_item_t *)callpath);
            }

            /* Set scope */
            if (NULL != xmlGetProp(node, "s")) {
                callpath->scope = atoi(xmlGetProp(node, "s"));
            }

            /* Alien? Terrestrial? E.T.? Home! */
            if (NULL != xmlGetProp(node, "a")) {
                callpath->alien = atoi(xmlGetProp(node, "a"));
            }

            OUTPUT_VERBOSE((10,
                "   %s i=[%d] s=[%d] a=[%d] n=[%s] f=[%s] l=[%d] lm=[%s]",
                _RED("call"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, callpath->procedure->file->shortname,
                callpath->procedure->line,
                callpath->procedure->module->shortname));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (L)oop */
        if (!xmlStrcmp(node->name, (const xmlChar *)"L")) {
            loop_t *hotspot = NULL;
            loop_t *loop = NULL;
            char *name = NULL;
            int temp_int = 0;

            loopdepth++;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "s")) ||
                (NULL == xmlGetProp(node, "i")) ||
                (NULL == xmlGetProp(node, "l"))) {
                OUTPUT(("%s", _ERROR("Error: malformed loop")));
            }

            /* Alocate callpath and initialize it */
            PERFEXPERT_ALLOC(callpath_t, callpath, sizeof(callpath_t));
            callpath->alien = parent->alien;
            callpath->parent = parent->parent;
            callpath->id = atoi(xmlGetProp(node, "i"));
            callpath->scope = atoi(xmlGetProp(node, "s"));
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Add callpath to paren'ts list of callees */
            perfexpert_list_append(&(parent->callees),
                (perfexpert_list_item_t *)callpath);

            /* Generate unique loop name (I know, it could be not unique) */
            if (PERFEXPERT_HOTSPOT_LOOP == parent->procedure->type) {
                PERFEXPERT_ALLOC(char, name, (strlen(parent->procedure->name) +
                    strlen(xmlGetProp(node, "l")) + 6));
                sprintf(name, "%s_loop%s", parent->procedure->name,
                    xmlGetProp(node, "l"));
            } else {
                PERFEXPERT_ALLOC(char, name,
                    (strlen(parent->procedure->module->shortname) +
                    strlen(parent->procedure->file->shortname) +
                    strlen(parent->procedure->name) +
                    strlen(xmlGetProp(node, "l")) + 9));
                sprintf(name, "%s_%s_%s_loop%s",
                    parent->procedure->module->shortname,
                    parent->procedure->file->shortname, parent->procedure->name,
                    xmlGetProp(node, "l"));
            }

            /* Check if this loop is already in the list of hotspots */
            hotspot = (loop_t *)perfexpert_list_get_first(&(profile->hotspots));
            while (((perfexpert_list_item_t *)hotspot !=
                &(profile->hotspots.sentinel)) && (NULL == loop)) {
                if ((0 == strcmp(hotspot->name, name)) &&
                    (PERFEXPERT_HOTSPOT_LOOP == hotspot->type)) {
                    loop = hotspot;
                }
                hotspot = (loop_t *)perfexpert_list_get_next(hotspot);
            }

            if (NULL == loop) {
                OUTPUT_VERBOSE((10, "   --- New loop found"));

                /* Allocate loop and set properties */
                PERFEXPERT_ALLOC(loop_t, loop, sizeof(loop_t));
                loop->name = name;
                loop->valid = PERFEXPERT_TRUE;
                loop->cycles = 0.0;
                loop->variance = 0.0;
                loop->importance = 0.0;
                loop->instructions = 0.0;
                loop->type = PERFEXPERT_HOTSPOT_LOOP;
                loop->id = atoi(xmlGetProp(node, "i"));
                loop->line = atoi(xmlGetProp(node, "l"));

                if (PERFEXPERT_HOTSPOT_FUNCTION == parent->procedure->type) {
                    loop->procedure = parent->procedure;
                } else {
                    loop_t *parent_loop = (loop_t *)parent->procedure;
                    loop->procedure = parent_loop->procedure;
                }

                loop->lcpi_by_name = NULL;
                loop->metrics_by_id = NULL;
                loop->metrics_by_name = NULL;
                loop->depth = loopdepth;
                perfexpert_list_construct(&(loop->metrics));
                perfexpert_list_item_construct((perfexpert_list_item_t *)loop);

                /* Add loop to list of hotspots */
                perfexpert_list_append(&(profile->hotspots),
                    (perfexpert_list_item_t *)loop);
            }

            /* Set procedure */
            callpath->procedure = (procedure_t *)loop;

            OUTPUT_VERBOSE((10,
                "   %s i=[%d] s=[%d], a=[%d] n=[%s] p=[%s] f=[%s] l=[%d] lm=[%s] d=[%d]",
                _GREEN("loop"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, loop->procedure->name,
                loop->procedure->file->shortname, callpath->procedure->line,
                loop->procedure->module->shortname, loopdepth));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (M)etric */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"M"))) {
            int temp_int = 0;
            metric_t *metric_entry = NULL;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "n")) || 
                (NULL == xmlGetProp(node, "v"))) {
                OUTPUT(("%s", _ERROR("Error: malformed metric")));
            }

            /* Are we in the callpath? */
            if (NULL == parent) {
                OUTPUT(("%s", _ERROR("Error: metric outside callpath")));
                return PERFEXPERT_ERROR;                
            }

            /* Find metric */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profile->metrics_by_id, &temp_int,
                metric_entry);
            if (NULL == metric_entry) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown metric")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate metric and initialize it */
            PERFEXPERT_ALLOC(metric_t, metric, sizeof(metric_t));
            metric->id = metric_entry->id;
            metric->name = metric_entry->name;
            metric->thread = metric_entry->thread;
            metric->mpi_rank = metric_entry->mpi_rank;
            metric->experiment = metric_entry->experiment;
            metric->value = atof(xmlGetProp(node, "v"));
            strcpy(metric->name_md5, metric_entry->name_md5);
            perfexpert_list_item_construct((perfexpert_list_item_t *)metric);
            perfexpert_list_append(&(parent->procedure->metrics),
                (perfexpert_list_item_t *)metric);

            OUTPUT_VERBOSE((10,
                "   %s [%d] of [%d] (%s) (%g) [rank=%d] [tid=%d] [exp=%d]",
                _CYAN("metric value"), metric->id, parent->id, metric->name,
                metric->value, metric->mpi_rank, metric->thread,
                metric->experiment));
        }

        /* LoadModuleTable, SecHeader, SecCallPathProfileData, MetricTable */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"LoadModuleTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecHeader")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfileData")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"MetricTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"FileTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"ProcedureTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"S")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"C"))) {
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parsing children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Move to the next node */
        node = node->next;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
