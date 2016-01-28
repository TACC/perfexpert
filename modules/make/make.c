/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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

/* Modules headers */
#include "make.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"

/* run_make */
int run_make(void) {
    char temp_str[MAX_BUFFER_SIZE];
    char flags[MAX_BUFFER_SIZE];
    char *argv[MAX_ARGUMENTS_COUNT];
    test_t test;
    int argc = 0, i = 0;
    int rc;


    OUTPUT_VERBOSE((5, "%s", _BLUE("Running 'make'")));

    /* If the user chose a Makefile... */
    if ((PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./Makefile")) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./makefile")) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./GNUmakefile"))) {
        OUTPUT(("%s",
        	_ERROR("'Makefile', 'makefile', or 'GNUmakefile' file not found")));
        return PERFEXPERT_ERROR;
    }

    argv[argc] = "make";
    argc++;
    if (my_module_globals.args[0]) {
        i = 0;
        while (NULL != my_module_globals.args[i]){
            argv[argc] = my_module_globals.args[i];
            i++;
            argc++;
        }
    }
    argv[argc] = NULL;
    argc++;

    if (NULL != getenv("CFLAGS")) {
        strcat(flags, getenv("CFLAGS"));
        strcat(flags, " ");
    }
    if (NULL != getenv("PERFEXPERT_CFLAGS")) {
        strcat(flags, getenv("PERFEXPERT_CFLAGS"));
    }
    setenv("CFLAGS", flags, 1);

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i = 0;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        while (NULL != argv[i]) {
            printf(" %s", argv[i]);
            i++;
        }
        printf("\n");
    }

    /* Fill the ninja test structure... */
    test.info = globals.program;
    test.input = NULL;
    bzero(temp_str, MAX_BUFFER_SIZE);
    sprintf(temp_str, "%s/make.output", globals.moduledir);
    test.output = temp_str;

    /* fork_and_wait_and_pray */
    rc = perfexpert_fork_and_wait(&test, argv);

    switch (rc) {
        case PERFEXPERT_NO_REC:
        case PERFEXPERT_FAILURE:
        case PERFEXPERT_ERROR:
            OUTPUT(("%s (return code: %d) Usually, this means that an error"
                " happened during the program execution. To see the program"
                "'s output, check the content of this file: [%s]. If you "
                "want to PerfExpert ignore the return code next time you "
                "run this program, set the 'return-code' option for the "
                "macpo module. See 'perfepxert -H macpo' for details.",
                _ERROR("the target program returned non-zero"), rc, 
                test.output));
            return PERFEXPERT_ERROR;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
            break;

        default:
            return PERFEXPERT_ERROR;
            break;
    }   

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
