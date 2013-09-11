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

/* PerfExpert headers */
#include "config.h"
#include "ct.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "perfexpert_fork.h"
#include "perfexpert_log.h"

/* apply_transformations */
int apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation) {
    transformation_t *transformation;
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
            goto move_on;
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
                goto move_on;

            case PERFEXPERT_NO_PAT:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                    transformation->id, transformation->program));
                goto move_on;

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
        move_on:
        transformation = (transformation_t *)perfexpert_list_get_next(
            transformation);
    }
    return rc;
}

/* test_transformation */
int test_transformation(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation) {
    char *argv[12];
    char temp_str[BUFFER_SIZE];
    int rc;
    test_t *test;

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
    argv[1] = (char *)malloc(strlen("-f") + 1);
    bzero(argv[1], strlen("-f") + 1);
    sprintf(argv[1], "-f");
    argv[2] = fragment->function_name; 
    argv[3] = (char *)malloc(strlen("-l") + 1);
    bzero(argv[3], strlen("-l") + 1);
    sprintf(argv[3], "-l");
    argv[4] = (char *)malloc(10);
    bzero(argv[4], 10);
    sprintf(argv[4], "%d", fragment->line_number); 
    argv[5] = (char *)malloc(strlen("-r") + 1);
    bzero(argv[5], strlen("-r") + 1);
    sprintf(argv[5], "-r");
    argv[6] = (char *)malloc(strlen(fragment->filename) + 5);
    bzero(argv[6], strlen(fragment->filename) + 5);
    sprintf(argv[6], "%s_new", fragment->filename);
    argv[7] = (char *)malloc(strlen("-s") + 1);
    bzero(argv[7], strlen("-s") + 1);
    sprintf(argv[7], "-s");
    argv[8] = fragment->filename;
    argv[9] = (char *)malloc(strlen("-w") + 1);
    bzero(argv[9], strlen("-w") + 1);
    sprintf(argv[9], "-w");
    argv[10] = (char *)malloc(strlen("./") + 1);
    bzero(argv[10], strlen("./") + 1);
    sprintf(argv[10], "./");
    argv[11] = NULL;

    /* The main test */
    test = (test_t *)malloc(sizeof(test_t));
    if (NULL == test) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    test->info   = fragment->filename;
    test->input  = NULL;
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/%s/%s.%s.output", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, transformation->program);
    test->output = temp_str;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
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
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s.old_%d", fragment->filename, getpid());

    if (PERFEXPERT_SUCCESS == rc) {
        if (rename(fragment->filename, temp_str)) {
            return PERFEXPERT_ERROR;
        }
        if (rename(argv[6], fragment->filename)) {
            return PERFEXPERT_ERROR;
        }
        OUTPUT(("applying %s to line %d of file %s", transformation->program,
            fragment->line_number, fragment->filename));
        OUTPUT(("the original file was renamed to %s", temp_str));
    }

    /* TODO: Free memory */

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
