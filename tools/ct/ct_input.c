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
#include "ct.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"

/* parse_transformation_params */
int parse_transformation_params(perfexpert_list_t *fragments) {
    recommendation_t *recommendation = NULL;
    fragment_t *fragment = NULL;
    char buffer[BUFFER_SIZE];
    int input_line = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Parsing fragments")));

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, globals.inputfile_FP)) {
        node_t *node = NULL;

        input_line++;
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Is this line a new recommendation? */
        if (0 == strncmp("%", buffer, 1)) {
            OUTPUT_VERBOSE((5, "   (%d) --- %s", input_line,
                _GREEN("new bottleneck found")));

            /* Create a list item for this code fragment */
            PERFEXPERT_ALLOC(fragment_t, fragment, sizeof(fragment_t));
            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)fragment);
            perfexpert_list_construct(&(fragment->recommendations));
            perfexpert_list_append(fragments,
                (perfexpert_list_item_t *)fragment);
            fragment->fragment_file = NULL;
            fragment->outer_loop_fragment_file = NULL;
            fragment->outer_outer_loop_fragment_file = NULL;
            fragment->outer_loop_line_number = 0;
            fragment->outer_outer_loop_line_number = 0;

            continue;
        }

        PERFEXPERT_ALLOC(node_t, node, (sizeof(node_t) + strlen(buffer) + 1));
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            PERFEXPERT_ALLOC(char, fragment->filename,
                (strlen(node->value) + 1));
            strcpy(fragment->filename, node->value);
            OUTPUT_VERBOSE((10, "   (%d) filename: [%s]", input_line,
                fragment->filename));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            fragment->line_number = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) line number: [%d]", input_line,
                fragment->line_number));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            fragment->code_type = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) type: [%d]", input_line,
                fragment->code_type));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.function_name */
        if (0 == strncmp("code.function_name", node->key, 18)) {
            PERFEXPERT_ALLOC(char, fragment->function_name,
                (strlen(node->value) + 1));
            strcpy(fragment->function_name, node->value);
            OUTPUT_VERBOSE((10, "   (%d) function name: [%s]", input_line,
                fragment->function_name));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.loopdepth */
        if (0 == strncmp("code.loopdepth", node->key, 14)) {
            fragment->loop_depth = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) loop depth: [%d]", input_line,
                fragment->loop_depth));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.rowid */
        if (0 == strncmp("code.rowid", node->key, 10)) {
            fragment->rowid = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) row id: [%d]", input_line,
                fragment->rowid));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: recommender.recommendation_id */
        if (0 == strncmp("recommender.recommendation_id", node->key, 29)) {
            PERFEXPERT_ALLOC(recommendation_t, recommendation,
                sizeof(recommendation_t));
            perfexpert_list_item_construct(
                (perfexpert_list_item_t *)recommendation);
            perfexpert_list_append(&(fragment->recommendations),
                (perfexpert_list_item_t *)recommendation);

            recommendation->id = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%d]", input_line,
                _GREEN("recommendation id:"), recommendation->id));
            PERFEXPERT_DEALLOC(node);
            continue;
        }

        /* Should never reach this, unless there is garbage in the input file */
        OUTPUT_VERBOSE((4, "   (%d) %s (%s = %s)", input_line,
            _RED("ignored line"), node->key, node->value));
        PERFEXPERT_DEALLOC(node);
    }

    /* print a summary of 'fragments' */
    OUTPUT_VERBOSE((4, "   %s (%d)", _MAGENTA("bottleneck(s) found"),
        perfexpert_list_get_size(fragments)));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
