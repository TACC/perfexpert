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

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/* PerfExpert tool headers */
#include "ct_rose.hpp"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

/* recommenderTraversal::visit */
void recommenderTraversal::visit(SgNode *node) {
    Sg_File_Info *info = NULL;
    SgFunctionDefinition *function = NULL;
    SgForStatement *c_loop = NULL;
    SgNode *grandparent = NULL;
    SgNode *parent = NULL;
    int node_found = 0;

    info = node->get_file_info();

    /* Find code fragment for bottlenecks type 'loop' in C */
    if ((isSgForStatement(node)) && (info->get_line() == fragment->line)
        && (PERFEXPERT_HOTSPOT_LOOP == fragment->type)) {

        /* Found a C loop on the exact line number */
        OUTPUT_VERBOSE((8, "         [loop] %s (line %d)",
            _GREEN((char *)"found"), info->get_line()));

        /* Extract the loop fragment */
        if (PERFEXPERT_SUCCESS != output_fragment(node, info, fragment)) {
            OUTPUT(("%s", _ERROR((char *)"extracting fragment")));
            rc = PERFEXPERT_ERROR;
            return;
        }

        /* Save the fragment path and filename */
        PERFEXPERT_ALLOC(char, fragment->fragment_file, (strlen(globals.workdir)
            + strlen(FRAGMENTS_DIR) + strlen(fragment->file) + 15));
        sprintf(fragment->fragment_file, "%s/%s/%s_%d", globals.workdir,
            FRAGMENTS_DIR, fragment->file, fragment->line);

        /* What is the loop detph and who is parent node */
        if (2 <= fragment->depth) {
            parent = node->get_parent();

            /* It is a basic block. Who is this basic block's parent? */
            if (NULL != isSgBasicBlock(parent)) {
                parent = parent->get_parent();
            }

            /* Is it a for/do/while? */
            if (isSgForStatement(parent)) {
                info = parent->get_file_info();
                fragment->outer_loop_line = info->get_line();

                /* The parent is a loop */
                OUTPUT_VERBOSE((8, "         [parent loop] %s (line %d)",
                    _GREEN((char *)"found"), fragment->outer_loop_line));

                /* Extract the parent loop fragment */
                if (PERFEXPERT_SUCCESS != output_fragment(parent, info,
                    fragment)) {
                    OUTPUT(("%s", _ERROR((char *)"extracting fragment")));
                    rc = PERFEXPERT_ERROR;
                    return;
                }

                /* Save the fragment path and filename */
                PERFEXPERT_ALLOC(char, fragment->outer_loop_fragment_file,
                    (strlen(globals.workdir) + strlen(FRAGMENTS_DIR)
                    + strlen(fragment->file) + 15));
                sprintf(fragment->outer_loop_fragment_file, "%s/%s/%s_%d",
                    globals.workdir, FRAGMENTS_DIR, fragment->file,
                    fragment->outer_loop_line);

                /* What is the loop detph and who is the grandparent node */
                if (3 <= fragment->depth) {
                    grandparent = parent->get_parent();

                    /* It is a basic block. Who is this basic block's parent? */
                    if (NULL != isSgBasicBlock(grandparent)) {
                        grandparent = grandparent->get_parent();
                    }

                    /* Is it a for/do/while? */
                    if (isSgForStatement(grandparent)) {
                        info = grandparent->get_file_info();
                        fragment->outer_outer_loop_line = info->get_line();

                        /* The grandparent is a loop */
                        OUTPUT_VERBOSE((8, "   [grandparent loop] %s (line %d)",
                            _GREEN((char *)"found"),
                            fragment->outer_outer_loop_line));

                        /* Extract the parent loop fragment */
                        if (PERFEXPERT_SUCCESS != output_fragment(grandparent,
                            info, fragment)) {
                            OUTPUT(("%s",
                                _ERROR((char *)"extracting fragment")));
                            rc = PERFEXPERT_ERROR;
                            return;
                        }

                        /* Save the fragment path and filename */
                        PERFEXPERT_ALLOC(char,
                            fragment->outer_outer_loop_fragment_file,
                            (strlen(globals.workdir) + strlen(FRAGMENTS_DIR) +
                            strlen(fragment->file) + 15));
                        sprintf(fragment->outer_outer_loop_fragment_file,
                            "%s/%s/%s_%d", globals.workdir, FRAGMENTS_DIR,
                            fragment->file, fragment->outer_outer_loop_line);
                    }
                }
            }
        }
    }

    /* If it is a function extract it! */
    if (NULL != (function = isSgFunctionDefinition(node))) {
        SgName function_name = function->get_declaration()->get_name();

        if (0 == strcmp(function_name.str(), fragment->name)) {
            /* Found a function with the rigth name */
            OUTPUT_VERBOSE((8, "         [%s] %s (line %d) ",
                fragment->name, _GREEN((char *)"found"), info->get_line()));

            /* Extract the function */
            if (PERFEXPERT_SUCCESS != output_function(node, fragment)) {
                OUTPUT(("%s", _ERROR((char *)"extracting function")));
                rc = PERFEXPERT_ERROR;
                return;
            }
        }
    }
}

/* recommenderTraversal::atTraversalStart */
void recommenderTraversal::atTraversalStart() {
    OUTPUT_VERBOSE((9, "      %s (%s)", _YELLOW((char *)"starting traversal"),
        fragment->file));
}

/* recommenderTraversal::atTraversalEnd */
void recommenderTraversal::atTraversalEnd() {
    OUTPUT_VERBOSE((9, "      %s (%s)", _YELLOW((char *)"ending traversal"),
        fragment->file));
}

// EOF
