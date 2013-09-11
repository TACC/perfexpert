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

/* apply_patterns */
int apply_patterns(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation) {
    pattern_t *pattern;
    int rc = PERFEXPERT_FAILURE;

    /* Return when there is no patterns for this transformation */
    if (0 == perfexpert_list_get_size(&(transformation->patterns))) {
        return PERFEXPERT_NO_PAT;
    }

    /* Did HPCToolkit identified the file */
    if (0 == strcmp("~unknown-file~", fragment->filename)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: unknown source file"),
            fragment->filename));
        return PERFEXPERT_NO_PAT;        
    }

    /* First of all, check if file exists */
    if (-1 == access(fragment->filename, R_OK )) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: source file not found"),
            fragment->filename));
        return PERFEXPERT_NO_PAT;
    }

    /* For each pattern required for this transfomation ... */
    pattern = (pattern_t *)perfexpert_list_get_first(
        &(transformation->patterns));
    while ((perfexpert_list_item_t *)pattern != 
        &(transformation->patterns.sentinel)) {

        /* Extract fragments, for that we have to open ROSE */
        if (PERFEXPERT_ERROR == open_rose(fragment->filename)) {
            OUTPUT(("%s", _RED("Error: starting Rose")));
            return PERFEXPERT_FAILURE;
        } else {
            /* Hey ROSE, here we go... */
            if (PERFEXPERT_ERROR == extract_fragment(fragment)) {
                OUTPUT(("%s (%s:%d)", _ERROR("Error: extracting fragments for"),
                    fragment->filename, fragment->line_number));
            }
            /* Close Rose */
                if (PERFEXPERT_SUCCESS != close_rose()) {
                OUTPUT(("%s", _ERROR("Error: closing Rose")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Test patterns */
        switch (test_pattern(fragment, recommendation, transformation,
                             pattern)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s] [%d] (%s)", _BOLDYELLOW("ERROR"),
                    pattern->id, pattern->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "   [%s ] [%d] (%s)", _BOLDRED("FAIL"),
                    pattern->id, pattern->program));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((7, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                    pattern->id, pattern->program));
               rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }

        /* Move to the next pattern */
        pattern = (pattern_t *)perfexpert_list_get_next(pattern);
    }
    return rc;
}

/* test_pattern */
int test_pattern(fragment_t *fragment, recommendation_t *recommendation,
    transformation_t *transformation, pattern_t *pattern) {
    perfexpert_list_t tests;
    test_t *test;
    int  rc = PERFEXPERT_FAILURE;
    char *temp_str;
    char *argv[2];

    argv[0] = pattern->program;
    argv[1] = NULL;

    /* Considering the outer loops, we can have more than one test */
    perfexpert_list_construct(&tests);

    /* The main test */
    test = (test_t *)malloc(sizeof(test_t));
    if (NULL == test) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)test);
    perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
    test->info  = fragment->fragment_file;
    test->input = fragment->fragment_file;
    temp_str = (char *)malloc(strlen(fragment->fragment_file) +
        strlen(pattern->program) + strlen(".output") + 2);
    if (NULL == temp_str) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(temp_str, strlen(fragment->fragment_file) + strlen(pattern->program) +
        strlen(".output") + 2);
    sprintf(temp_str, "%s.%s.output", fragment->fragment_file,
        pattern->program);
    test->output = temp_str;

    /* It we're testing for a loop, check for the outer loop */
    if ((0 == strncmp("loop", fragment->code_type, 4)) &&
        (2 <= fragment->loop_depth) &&
        (NULL != fragment->outer_loop_fragment_file)) {

        /* The outer loop test */
        test = (test_t *)malloc(sizeof(test_t));
        if (NULL == test) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        perfexpert_list_item_construct((perfexpert_list_item_t *)test);
        perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
        test->info  = fragment->outer_loop_fragment_file;
        test->input = fragment->outer_loop_fragment_file;
        temp_str = (char *)malloc(strlen(fragment->outer_loop_fragment_file) +
            strlen(pattern->program) + strlen(".output") + 2);
        if (NULL == temp_str) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(temp_str, strlen(fragment->outer_loop_fragment_file) +
            strlen(pattern->program) + strlen(".output") + 2);
        sprintf(temp_str, "%s.%s.output", fragment->outer_loop_fragment_file,
            pattern->program);
        test->output = temp_str;

        /* And test for the outer outer loop too */
        if ((3 <= fragment->loop_depth) &&
            (NULL != fragment->outer_outer_loop_fragment_file)) {

            /* The outer outer loop test */
            test = (test_t *)malloc(sizeof(test_t));
            if (NULL == test) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *)test);
            perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
            test->info  = fragment->outer_outer_loop_fragment_file;
            test->input = fragment->outer_outer_loop_fragment_file;
            temp_str = (char *)malloc(
                strlen(fragment->outer_outer_loop_fragment_file) +
                strlen(pattern->program) + strlen(".output") + 2);
            if (NULL == temp_str) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(temp_str, strlen(fragment->outer_outer_loop_fragment_file) +
                strlen(pattern->program) + strlen(".output") + 2);
            sprintf(temp_str, "%s.%s.output",
                fragment->outer_outer_loop_fragment_file, pattern->program);
            test->output = temp_str;
            }
    }

    /* Run all the tests */
    test = (test_t *)perfexpert_list_get_first(&tests);
    while ((perfexpert_list_item_t *)test != &(tests.sentinel)) {
        switch (fork_and_wait(test, argv)) {
            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            case PERFEXPERT_UNDEFINED:
            case PERFEXPERT_FAILURE:
            default:
                break;
        }
        test = (test_t *)perfexpert_list_get_next(
            (perfexpert_list_item_t *)test);
    }

    /* TODO: Free memory */

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
