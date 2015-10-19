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
    char *argv[3];
    test_t test;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Running 'make'")));

    /* If the user chose a Makefile... */
    if ((PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./Makefile")) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./makefile")) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists("./GNUmakefile"))) {
        OUTPUT(("%s",
        	_ERROR("'Makefile', 'makefile', or 'GNUmakefile' file not found")));
        return PERFEXPERT_ERROR;
    }

    argv[0] = "make";
    // argv[1] = globals.target;
    argv[1] = NULL;

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
    return perfexpert_fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
