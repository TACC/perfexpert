/*
 * Copyright (c) 2011-201666666  University of Texas at Austin. All rights reserved.
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
#include "icc.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"

/* run_icc */
int run_icc(void) {
    char *argv[MAX_ARGUMENTS_COUNT];
    char temp_str[MAX_BUFFER_SIZE];
    char flags[MAX_BUFFER_SIZE];
    int  argc = 0;
    test_t test;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Compiling the program")));
    OUTPUT(("%s [%s]", _YELLOW("Compiling"), globals.program));

    /*  if the user is passing something in CC, use it */
    if (NULL != getenv("CC")) {
        argv[0] = getenv("CC");
    }
    else {
        argv[0] = "icc";
    }
    argv[1] = "-o";
    argv[2] = globals.program;
    argv[3] = my_module_globals.source;

    /* What are the default and user defined compiler flags? */
    bzero(flags, MAX_BUFFER_SIZE);
    strcat(flags, DEFAULT_CFLAGS);
    if (NULL != getenv("CFLAGS")) {
        strcat(flags, getenv("CFLAGS"));
        strcat(flags, " ");
    }
    if (NULL != getenv("PERFEXPERT_CFLAGS")) {
        strcat(flags, getenv("PERFEXPERT_CFLAGS"));
    }

    argc = 4;
    argv[argc] = strtok(flags, " ");
    do {
        argc++;
    } while (argv[argc] = strtok(NULL, " "));

    argv[argc] = NULL;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i] ? argv[i] : "(null)");
        }
        printf("\n");
    }

    /* Fill the ninja test structure... */
    test.info = globals.program;
    test.input = NULL;
    bzero(temp_str, MAX_BUFFER_SIZE);
    sprintf(temp_str, "%s/icc.output", globals.moduledir);
    test.output = temp_str;

    /* fork_and_wait_and_pray */
    return perfexpert_fork_and_wait(&test, argv);
}

#ifdef __cplusplus
}
#endif

// EOF
