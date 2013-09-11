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

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/* PerfExpert headers */
#include "config.h"
#include "ct_rose.hpp"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

void recommenderTraversal::visit(SgNode *node) {
    Sg_File_Info *info = NULL;
    SgFunctionDefinition *function = NULL;
    SgForStatement *c_loop = NULL;
    SgNode *grandparent = NULL;
    SgNode *parent = NULL;
    int node_found = 0;

    info = node->get_file_info();

    /* Find code fragment for bottlenecks type 'loop' in C */
    if ((isSgForStatement(node)) && (info->get_line() == fragment->line_number)
        && (0 == strncmp("loop", fragment->code_type, 4))) {

        /* Found a C loop on the exact line number */
        OUTPUT_VERBOSE((8, "   %s (%d)", _GREEN((char *)"loop found at line"),
            info->get_line()));

        /* Extract the loop fragment */
        if (PERFEXPERT_SUCCESS != output_fragment(node, info, fragment)) {
            OUTPUT(("%s", _ERROR((char *)"Error: extracting fragment")));
            rc = PERFEXPERT_ERROR;
            return;
        }

        /* Save the fragment path and filename */
        fragment->fragment_file = (char *)malloc(strlen(globals.workdir) +
            strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 15);

        if (NULL == fragment->fragment_file) {
            OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
            rc = PERFEXPERT_ERROR;
            return;
        }

        bzero(fragment->fragment_file, strlen(globals.workdir) +
            strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 10);

        sprintf(fragment->fragment_file, "%s/%s/%s_%d", globals.workdir,
            PERFEXPERT_FRAGMENTS_DIR, fragment->filename,
            fragment->line_number);

        /* What is the loop detph and who is parent node */
        if (2 <= fragment->loop_depth) {
            parent = node->get_parent();

            /* It is a basic block. Who is this basic block's parent? */
            if (NULL != isSgBasicBlock(parent)) {
                parent = parent->get_parent();
            }

            /* Is it a for/do/while? */
            if (isSgForStatement(parent)) {
                info = parent->get_file_info();
                fragment->outer_loop_line_number = info->get_line();

                /* The parent is a loop */
                OUTPUT_VERBOSE((8, "   %s (%d)",
                    _GREEN((char *)"loop has a parent loop at line"),
                    fragment->outer_loop_line_number));

                /* Extract the parent loop fragment */
                if (PERFEXPERT_SUCCESS != output_fragment(parent, info,
                    fragment)) {
                    OUTPUT(("%s",
                        _ERROR((char *)"Error: extracting fragment")));
                    rc = PERFEXPERT_ERROR;
                    return;
                }

                /* Save the fragment path and filename */
                fragment->outer_loop_fragment_file = (char *)malloc(
                    strlen(globals.workdir) + strlen(PERFEXPERT_FRAGMENTS_DIR)
                    + strlen(fragment->filename) + 15);

                if (NULL == fragment->outer_loop_fragment_file) {
                    OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
                    rc = PERFEXPERT_ERROR;
                    return;
                }

                bzero(fragment->outer_loop_fragment_file,
                    strlen(globals.workdir) + strlen(PERFEXPERT_FRAGMENTS_DIR) +
                    strlen(fragment->filename) + 10);

                sprintf(fragment->outer_loop_fragment_file, "%s/%s/%s_%d",
                    globals.workdir, PERFEXPERT_FRAGMENTS_DIR,
                    fragment->filename, fragment->outer_loop_line_number);

                /* What is the loop detph and who is the grandparent node */
                if (3 <= fragment->loop_depth) {
                    grandparent = parent->get_parent();

                    /* It is a basic block. Who is this basic block's parent? */
                    if (NULL != isSgBasicBlock(grandparent)) {
                        grandparent = grandparent->get_parent();
                    }

                    /* Is it a for/do/while? */
                    if (isSgForStatement(grandparent)) {
                        info = grandparent->get_file_info();
                        fragment->outer_outer_loop_line_number = 
                            info->get_line();

                        /* The grandparent is a loop */
                        OUTPUT_VERBOSE((8, "   %s (%d)",
                            _GREEN((char *)"loop has grandparent loop at line"),
                            info->get_line()));

                        /* Extract the parent loop fragment */
                        if (PERFEXPERT_SUCCESS != output_fragment(grandparent,
                            info, fragment)) {
                            OUTPUT(("%s",
                                _ERROR((char *)"Error: extracting fragment")));
                            rc = PERFEXPERT_ERROR;
                            return;
                        }

                        /* Save the fragment path and filename */
                        fragment->outer_outer_loop_fragment_file =
                            (char *)malloc(strlen(globals.workdir) +
                            strlen(PERFEXPERT_FRAGMENTS_DIR) +
                            strlen(fragment->filename) + 15);

                        if (NULL == fragment->outer_outer_loop_fragment_file) {
                            OUTPUT(("%s",
                                _ERROR((char *)"Error: out of memory")));
                            rc = PERFEXPERT_ERROR;
                            return;
                        }

                        bzero(fragment->outer_outer_loop_fragment_file,
                            strlen(globals.workdir) +
                            strlen(PERFEXPERT_FRAGMENTS_DIR) +
                            strlen(fragment->filename) + 10);

                        sprintf(fragment->outer_outer_loop_fragment_file,
                            "%s/%s/%s_%d", globals.workdir,
                            PERFEXPERT_FRAGMENTS_DIR, fragment->filename,
                            fragment->outer_outer_loop_line_number);
                    }
                }
            }
        }
    }

    /* If it is a function extract it! */
    if (NULL != (function = isSgFunctionDefinition(node))) {
        SgName function_name = function->get_declaration()->get_name();

        if (0 == strcmp(function_name.str(), fragment->function_name)) {
            /* Found a function with the rigth name */
            OUTPUT_VERBOSE((8, "   %s (%d) [%s]",
                _GREEN((char *)"function found at line"), info->get_line(),
                fragment->function_name));

            /* Extract the function */
            if (PERFEXPERT_SUCCESS != output_function(node, fragment)) {
                OUTPUT(("%s", _ERROR((char *)"Error: extracting function")));
                rc = PERFEXPERT_ERROR;
                return;
            }
        }
    }
}

void recommenderTraversal::atTraversalStart() {
    OUTPUT_VERBOSE((9, "   %s (%s)", _YELLOW((char *)"starting traversal on"),
        fragment->filename));
}

void recommenderTraversal::atTraversalEnd() {
    OUTPUT_VERBOSE((9, "   %s (%s)", _YELLOW((char *)"ending traversal on"),
        fragment->filename));
}

// EOF
