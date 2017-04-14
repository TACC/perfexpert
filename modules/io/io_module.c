/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <float.h>
#include <time.h>

/* Module headers */
#include "io_module.h"
#include "io.h"
#include "io_database.h"
#include "io_output.h"

        /* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_time.h"
#include "common/perfexpert_util.h"

/* Global variable to define the module itself */
perfexpert_module_io_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    int i;
    /* Module pre-requisites */

    /* Initialize some variables */
    my_module_globals.inputfile = NULL;
    my_module_globals.prefix[0] = NULL;
    my_module_globals.before[0] = NULL;
    my_module_globals.after[0] = NULL;
    my_module_globals.ignore_return_code = PERFEXPERT_FALSE;

    for (i=0; i<MAX_FUNCTIONS; ++i) {
        my_module_globals.data[i].size=0;
    }

    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing module arguments")));
        return PERFEXPERT_ERROR;
    }   

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));
    myself_module.status = PERFEXPERT_MODULE_INITIALIZED;

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    int i;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    for (i =0; i < MAX_FUNCTIONS; ++i) {
        PERFEXPERT_DEALLOC(my_module_globals.data[i].code);
    }
    return PERFEXPERT_SUCCESS;
}

/* module_measure */
int module_measure(void) {
    /* 
       Run the code.
       We need to use LD_PRELOAD to access the wrappers implemented in io_wrapper.c
    */
    experiment_t *e;
    int i, rc, argc;
    struct timespec time_start, time_end, time_diff;
    test_t test;
   
    if (PERFEXPERT_SUCCESS !=
        perfexpert_util_file_is_exec(globals.program_full)) {
        return PERFEXPERT_ERROR;
    }

    /* Run the BEFORE program */
    if (NULL != my_module_globals.before[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir)+20));
        sprintf(test.output, "%s/before.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.before[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.before)) {
            OUTPUT(("   %s", _RED("'before' command returns non-zero")));
        }   
        PERFEXPERT_DEALLOC(test.output);
    }  

    /* Create a new experiment */
    PERFEXPERT_ALLOC(experiment_t, e, sizeof(experiment_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)e);
    e->argc = 0;

    e->argv[e->argc] = "LD_PRELOAD=/work/02658/agomez/tools/perfexpert/modules/io/.libs/libperfexpert_module_io_wrapper.so";
    e->argc++;
    PERFEXPERT_ALLOC(char, e->argv[e->argc], strlen(globals.program_full)+20);
    sprintf(e->argv[e->argc], "PERFEXPERT_PROGRAM=%s", globals.program_full);
    e->argc++;

    sprintf(e->argv[e->argc], "PERFEXPERT_IO_FOLDER=%s", globals.moduledir);
    e->argc++;

    argc = 0;
    while (NULL != my_module_globals.prefix[argc]) {
        e->argv[e->argc] = my_module_globals.prefix[argc];
        argc++;
        e->argc++;
    }

    e->argv[e->argc] = globals.program_full;
    e->argc++;

    i=0;
    while (NULL != globals.program_argv[i]) {
        e->argc[e->argv] = globals.program_argv[i];
        e->argc++;
        i++;
    }  
    e->argv[e->argc] = NULL;
    PERFEXPERT_ALLOC(char, e->test.output,
        (strlen(globals.moduledir) + strlen(IO) + 25));

    sprintf(e->test.output, "%s/%s.output", globals.moduledir, IO);
    e->test.input = my_module_globals.inputfile;
    e->test.info = globals.program;

    if (4 <= globals.verbose) {
        printf("%s %s", PROGRAM_PREFIX, _YELLOW("command line:"));                                                               
        for (i = 0; i < e->argc; i++) {
            printf(" %s", e->argv[i]);
        }
        printf("\n");
    }

    OUTPUT(("%s", _YELLOW("Running the code")));
    /*  Fork and run the command */
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    rc = perfexpert_fork_and_wait(&(e->test), (char **)e->argv);
    clock_gettime(CLOCK_MONOTONIC, &time_end);

    perfexpert_time_diff(&time_diff, &time_start, &time_end);
    OUTPUT(("   %lld.%.9ld seconds (includes measurement overhead)",
        (long long)time_diff.tv_sec, time_diff.tv_nsec));

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
                        " option for the IO module. See 'perfexpert -H "
                        " hpctoolkit' for details.",
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
        sprintf(test.output, "%s/after.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.after[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.after)) {
            OUTPUT(("%s", _RED("'after' command return non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    return rc;
}

/* module_analyze */
int module_analyze(void) {
    char path[255];

    OUTPUT(("%s", _YELLOW("Analysing measurements")));

    snprintf (path, 255, "%s/perfexpert_io_output", globals.moduledir);

    if (PERFEXPERT_SUCCESS != generate_raw_output(path, globals.program_full)) {
        OUTPUT(("%s", _ERROR("processing output")));
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != database_export(my_module_globals.data)) {
        OUTPUT(("%s", _ERROR("storing data into database")));
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != output_analysis()) {
        OUTPUT(("%s", _ERROR("printing analysis report")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
