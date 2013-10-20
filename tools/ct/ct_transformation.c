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

/* PerfExpert headers */
#include "ct.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_fork.h"
#include "perfexpert_list.h"
#include "perfexpert_log.h"
#include "perfexpert_output.h"

/* apply_transformations */
int apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation) {
    transformation_t *transformation = NULL;
    int rc = PERFEXPERT_NO_TRANS;

    /* Return when there is no transformations for this recommendation */
    if (0 == perfexpert_list_get_size(&(recommendation->transformations))) {
        return PERFEXPERT_NO_TRANS;
    }

    /* For each transformation for this recommendation... */
    transformation = (transformation_t *)perfexpert_list_get_first(
        &(recommendation->transformations));
    while ((perfexpert_list_item_t *)transformation !=
        &(recommendation->transformations.sentinel)) {

        /* Skip if other transformation has already been applied */
        if (PERFEXPERT_SUCCESS == rc) {
            OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                transformation->id, transformation->program));
            goto MOVE_ON;
        }

        /* Apply patterns */
        switch (apply_patterns(fragment, recommendation, transformation)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _BOLDYELLOW("ERROR"),
                    transformation->id, transformation->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _BOLDRED("FAIL"),
                    transformation->id, transformation->program));
                goto MOVE_ON;

            case PERFEXPERT_NO_PAT:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                    transformation->id, transformation->program));
                goto MOVE_ON;

            case PERFEXPERT_SUCCESS:
            default:
                break;
        }

        /* Test transformation */
        switch (test_transformation(fragment, recommendation, transformation)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _YELLOW("ERROR"),
                    transformation->id, transformation->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _RED("FAIL"),
                    transformation->id, transformation->program));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((8, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                    transformation->id, transformation->program));
                rc = PERFEXPERT_SUCCESS;
                LOG(("transformation=%s", transformation->program));
                break;

            default:
                break;
        }

        /* Move to the next code transformation */
        MOVE_ON:
        transformation = (transformation_t *)perfexpert_list_get_next(
            transformation);
    }
    return rc;
}

/* test_transformation */
int test_transformation(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation) {
    int rc = PERFEXPERT_SUCCESS;
    char *new_file = NULL;
    char *argv[12];
    test_t *test = NULL;

    /* Set the code transformer arguments. Ok, we have to define an
     * interface to code transformers. Here is a simple one. Each code
     * transformer will be called using the following arguments:
     *
     * -f FUNCTION  Function name were code bottleneck belongs to
     * -l LINE      Line number identified by HPCtoolkit/PerfExpert/etc...
     * -r FILE      File to write the transformation result
     * -s FILE      Source file
     * -w DIR       Use DIR as work directory
     */
    argv[0] = transformation->program;
    argv[1] = "-f";
    argv[2] = fragment->function_name; 
    argv[3] = "-l";
    PERFEXPERT_ALLOC(char, argv[4], 10);
    sprintf(argv[4], "%d", fragment->line_number); 
    argv[5] = "-r";
    PERFEXPERT_ALLOC(char, argv[6], (strlen(fragment->filename) + 5));
    sprintf(argv[6], "%s_new", fragment->filename);
    argv[7] = "-s";
    argv[8] = fragment->filename;
    argv[9] = "-w";
    argv[10] = "./";
    argv[11] = NULL;

    /* The main test */
    PERFEXPERT_ALLOC(test_t, test, sizeof(test_t));
    PERFEXPERT_ALLOC(char, test->output,
        (strlen(globals.workdir) + strlen(PERFEXPERT_FRAGMENTS_DIR) +
            strlen(fragment->filename) + strlen(transformation->program) + 11));
    sprintf(test->output, "%s/%s/%s.%s.output", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, transformation->program);
    test->input = NULL;
    test->info = fragment->filename;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 11; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* fork_and_wait_and_pray */
    rc = fork_and_wait(test, argv);

    /* Replace the source code file */
    PERFEXPERT_ALLOC(char, new_file, (strlen(fragment->filename) + 10));
    sprintf(new_file, "%s.old_%d", fragment->filename, getpid());

    if (PERFEXPERT_SUCCESS == rc) {
        if (rename(fragment->filename, new_file)) {
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }
        if (rename(argv[6], fragment->filename)) {
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }
        fprintf(globals.outputfile_FP, "%s was applied to line %d of %s\n",
            _CYAN(transformation->program), fragment->line_number,
            fragment->filename);
        fprintf(globals.outputfile_FP, "The original file was saved as %s\n",
            _MAGENTA(new_file));
    }

    /* Free memory */
    CLEAN_UP:
    PERFEXPERT_DEALLOC(argv[4]);
    PERFEXPERT_DEALLOC(argv[6]);
    PERFEXPERT_DEALLOC(test->output);
    PERFEXPERT_DEALLOC(new_file);

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
