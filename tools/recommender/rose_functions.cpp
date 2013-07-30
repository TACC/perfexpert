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
#include <unistd.h>
#include <string.h>

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "rose_functions.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;

extern globals_t globals; // globals was defined on 'recommender.c'

SgProject *userProject;

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

int open_rose(const char *source_file) {
    char **files = NULL;

    OUTPUT_VERBOSE((7, "=== %s", _BLUE((char *)"Opening Rose")));

    /* Fill 'files', aka **argv */
    files = (char **)malloc(sizeof(char *) * 3);

    files[0] = (char *)malloc(sizeof("recommender") + 1);
    if (NULL == files[0]) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(files[0], sizeof("recommender") + 1);
    snprintf(files[0], sizeof("recommender"), "recommender");

    files[1] = (char *)malloc(strlen(source_file) + 1);
    if (NULL == files[1]) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(files[1], strlen(source_file) + 1);
    strncpy(files[1], source_file, strlen(source_file));

    files[2] = NULL;

    /* Load files and build AST */
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);
    
    /* I believe now it is OK to free 'argv' */
    free(files[0]);
    free(files);

    return PERFEXPERT_SUCCESS;
}

int close_rose(void) {
    // TODO: should find a way to free 'userProject'
    OUTPUT_VERBOSE((7, "=== %s", _BLUE((char *)"Closing Rose")));

    return PERFEXPERT_SUCCESS;
}

int extract_fragment(segment_t *segment) {
    char *fragments_dir = NULL;
    recommenderTraversal segmentTraversal;

    OUTPUT_VERBOSE((7, "=== %s (%s:%d) [%s]",
                    _BLUE((char *)"Extracting fragment from"),
                    segment->filename, segment->line_number, segment->type));

    /* Create the fragments directory */
    fragments_dir = (char *)malloc(strlen(globals.workdir) +
                                   strlen(PERFEXPERT_FRAGMENTS_DIR) + 2);
    if (NULL == fragments_dir) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        exit(PERFEXPERT_ERROR);
    }
    bzero(fragments_dir, strlen(globals.workdir) +
          strlen(PERFEXPERT_FRAGMENTS_DIR) + 2);
    sprintf(fragments_dir, "%s/%s", globals.workdir, PERFEXPERT_FRAGMENTS_DIR);
    if (PERFEXPERT_ERROR == perfexpert_util_make_path(fragments_dir, 0755)) {
        OUTPUT(("%s", _ERROR((char *)"Error: cannot create fragments dir")));
        exit(PERFEXPERT_ERROR);
    } else {
        OUTPUT_VERBOSE((4, "fragments will be put in (%s)", fragments_dir));
    }
    free(fragments_dir);

    /* Build the traversal object and call the traversal function starting at
     * the project node of the AST, using a pre-order traversal
     */
    segmentTraversal.item = segment;
    segmentTraversal.traverseInputFiles(userProject, preorder);

    OUTPUT_VERBOSE((7, "==="));
    
    return PERFEXPERT_SUCCESS;
}

static int output_fragment(SgNode *node, Sg_File_Info *fileInfo,
                           segment_t *item) {
    char *fragment_file = NULL;
    FILE *fragment_file_FP;

    /* Set fragment filename */
    fragment_file = (char *)malloc(strlen(globals.workdir) +
                                   strlen(PERFEXPERT_FRAGMENTS_DIR) +
                                   strlen(item->filename) + 10);
    if (NULL == fragment_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(fragment_file, (strlen(globals.workdir) +
                          strlen(PERFEXPERT_FRAGMENTS_DIR) +
                          strlen(item->filename) + 10));
    sprintf(fragment_file, "%s/%s/%s_%d", globals.workdir,
            PERFEXPERT_FRAGMENTS_DIR, item->filename, fileInfo->get_line());
    OUTPUT_VERBOSE((8, "extracting it to (%s)", fragment_file));

    fragment_file_FP = fopen(fragment_file, "w+");
    if (NULL == fragment_file_FP) {
        OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
                _ERROR(fragment_file)));
        return PERFEXPERT_ERROR;
    }
    fprintf(fragment_file_FP, "%s", node->unparseToCompleteString().c_str());
    fclose(fragment_file_FP);
    free(fragment_file);

    return PERFEXPERT_SUCCESS;
}

void recommenderTraversal::visit(SgNode *node) {
    Sg_File_Info *fileInfo = NULL;
    Sg_File_Info *parent_info = NULL;
    Sg_File_Info *grand_parent_info = NULL;
    SgFunctionDefinition *function = NULL;
    SgForStatement *c_loop = NULL;
    SgForStatement *parent_loop = NULL;
    SgForStatement *grandparent_loop = NULL;
    SgNode *grandparent = NULL;
    SgNode *parent = NULL;
    int node_found = 0;

    fileInfo = node->get_file_info();

    /* Find code fragment for bottlenecks type 'loop' in C */
    if ((NULL != (c_loop = isSgForStatement(node))) &&
        (0 == strncmp("loop", item->type, 4)) &&
        (fileInfo->get_line() == item->line_number)) {
        //SgLabelStatement *label = NULL;
        //SgScopeStatement *scope = NULL;
        //SgStatement *statement = NULL;

        //char label_name[PERFEXPERT_LOOP_LABEL];
        //bzero(label_name, PERFEXPERT_LOOP_LABEL);
        //sprintf(label_name, "loop_%d", fileInfo->get_line());

        //SgName name = label_name;

        /* Found a C loop on the exact line number */
        OUTPUT_VERBOSE((8, "found a (%s) on (%s:%d)", node->sage_class_name(),
                        fileInfo->get_filename(), fileInfo->get_line()));

        /* Extract the loop fragment */
        if (PERFEXPERT_SUCCESS != output_fragment(node, fileInfo, item)) {
            OUTPUT(("%s", _ERROR((char *)"Error: extracting fragment")));
            return;
        }
        fprintf(globals.outputfile_FP, "recommender.code_fragment=%s/%s/%s_%d\n",
                globals.workdir, PERFEXPERT_FRAGMENTS_DIR, item->filename,
                item->line_number);

        /* What is the loop detph and who is node's parent */
        if (2 <= item->loop_depth) {

            parent = node->get_parent();

            /* It is a basic block. Who is this basic block's parent? */
            if (NULL != isSgBasicBlock(parent)) {
                parent = parent->get_parent();
            }

            /* Is it a for/do/while? */
            if (NULL != (parent_loop = isSgForStatement(parent))) {
                parent_info = parent->get_file_info();
                item->outer_loop = parent_info->get_line();

                /* The parent is a loop */
                OUTPUT_VERBOSE((8, "loop has a parent loop at (%s:%d)",
                                parent_info->get_filename(),
                                parent_info->get_line()));

                /* Extract the parent loop fragment */
                if (PERFEXPERT_SUCCESS != output_fragment(parent, parent_info,
                                                          item)) {
                    OUTPUT(("%s", _ERROR((char *)"Error: extracting fragment")));
                    return;
                }
                fprintf(globals.outputfile_FP, "code.outer_loop=%d\n",
                        item->outer_loop);
                fprintf(globals.outputfile_FP,
                        "recommender.outer_loop_fragment=%s/%s/%s_%d\n",
                        globals.workdir, PERFEXPERT_FRAGMENTS_DIR,
                        item->filename, parent_info->get_line());

                /* What is the loop detph and who is node's grandparent */
                if (3 <= item->loop_depth) {
                    grandparent = parent->get_parent();

                    /* It is a basic block. Who is this basic block's parent? */
                    if (NULL != isSgBasicBlock(grandparent)) {
                        grandparent = grandparent->get_parent();
                    }

                    /* Is it a for/do/while? */
                    if (NULL != (grandparent_loop = isSgForStatement(grandparent))) {
                        grand_parent_info = grandparent->get_file_info();
                        item->outer_outer_loop = grand_parent_info->get_line();

                        /* The grandparent is a loop */
                        OUTPUT_VERBOSE((8,
                                        "loop has a grandparent loop at (%s:%d)",
                                        grand_parent_info->get_filename(),
                                        grand_parent_info->get_line()));

                        /* Extract the parent loop fragment */
                        if (PERFEXPERT_SUCCESS != output_fragment(grandparent,
                                                                  grand_parent_info,
                                                                  item)) {
                            OUTPUT(("%s", _ERROR((char *)"Error: extracting fragment")));
                            return;
                        }
                        fprintf(globals.outputfile_FP,
                                "code.outer_outer_loop=%d\n",
                                item->outer_outer_loop);
                        fprintf(globals.outputfile_FP,
                                "recommender.outer_outer_loop_fragment=%s/%s/%s_%d\n",
                                globals.workdir, PERFEXPERT_FRAGMENTS_DIR,
                                item->filename, grand_parent_info->get_line());

                        /* Add a comment to the loop */
                        //attachComment(grandparent_loop,
                        //              "PERFEXPERT: start work here");
                        //attachComment(grandparent_loop,
                        //              "PERFEXPERT: grandparent loop of bottleneck");

                        /* Label the loop */
                        //label = NULL;
                        //scope = NULL;
                        //statement = NULL;

                        //bzero(label_name, PERFEXPERT_LOOP_LABEL);
                        //sprintf(label_name, "loop_%d", grand_parent_info->get_line());
                        //SgName name_grandparent = label_name;

                        //label = buildLabelStatement(name_grandparent, statement,
                        //                            scope);
                        //insertStatementBefore(isSgStatement(grandparent_loop),
                        //                      label);
                    }
                } else {
                //    attachComment(parent_loop, "PERFEXPERT: start work here");
                }
                /* Add a comment to the loop */
                //attachComment(parent_loop,
                //              "PERFEXPERT: parent loop of bottleneck");

                /* Label the loop */
                // label = NULL;
                // scope = NULL;
                // statement = NULL;

                // bzero(label_name, PERFEXPERT_LOOP_LABEL);
                // sprintf(label_name, "loop_%d", parent_info->get_line());
                // SgName name_parent = label_name;

                // label = buildLabelStatement(name_parent, statement, scope);
                // insertStatementBefore(isSgStatement(parent_loop), label);
            }
        } else {
            // attachComment(c_loop, "PERFEXPERT: start work here");
        }
        /* Add a comment to the loop */
        // attachComment(c_loop, "PERFEXPERT: bottleneck");

        /* Label the loop */
        // label = buildLabelStatement(name, statement, scope);
        // insertStatementBefore(isSgStatement(node), label);
    }

    /* Find code fragments for bottlenecks type 'function' */
    if ((NULL != (function = isSgFunctionDefinition(node))) &&
        (0 == strncmp("function", item->type, 8)) &&
        (fileInfo->get_line() == item->line_number)) {
        /* found a function on the exact line number */
        node_found = 1;
        // attachComment(function, "PERFEXPERT working here");
    }

    /* Extract code fragment */
    if (1 == node_found) {
        OUTPUT_VERBOSE((8, "found a (%s) on (%s:%d)", node->sage_class_name(),
                        fileInfo->get_filename(), fileInfo->get_line()));
        output_fragment(node, fileInfo, item);
    }
}

void recommenderTraversal::atTraversalStart() {
    OUTPUT_VERBOSE((9, "%s (%s)", _YELLOW((char *)"starting traversal on"),
                    item->filename));
}

void recommenderTraversal::atTraversalEnd() {
    OUTPUT_VERBOSE((9, "%s (%s)", _YELLOW((char *)"ending traversal on"),
                    item->filename));
}

// EOF
