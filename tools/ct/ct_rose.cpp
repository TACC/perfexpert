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
#include "perfexpert_util.h"
#include "perfexpert_log.h"
#include "perfexpert_base64.h"

/* Global variables, try to not create them! */
SgProject *userProject;

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

int open_rose(const char *source_file) {
    char **files = NULL;

    OUTPUT_VERBOSE((7, "   %s", _MAGENTA((char *)"opening Rose")));

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
    userProject = NULL;
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);
    
    /* I believe now it is OK to free 'argv' */
    free(files[0]);
    free(files);

    return PERFEXPERT_SUCCESS;
}

int close_rose(void) {
    // TODO: should find a way to free 'userProject'
    return PERFEXPERT_SUCCESS;
}

int extract_fragment(fragment_t *fragment) {
    char *fragments_dir = NULL;
    recommenderTraversal fragmentTraversal;

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
        OUTPUT_VERBOSE((4, "   fragments will be put in (%s)", fragments_dir));
    }
    free(fragments_dir);

    /* Build the traversal object and call the traversal function starting at
     * the project node of the AST, using a pre-order traversal
     */
    OUTPUT_VERBOSE((7, "   %s (%s:%d) [%s]",
        _YELLOW((char *)"extracting fragments from"), fragment->filename,
        fragment->line_number, fragment->code_type));

    fragmentTraversal.fragment = fragment;
    fragmentTraversal.traverseInputFiles(userProject, preorder);

    return PERFEXPERT_SUCCESS;
}

int output_fragment(SgNode *node, Sg_File_Info *info,
    fragment_t *fragment) {
    char *fragment_file = NULL;
    FILE *fragment_file_FP;

    /* Set fragment filename */
    fragment_file = (char *)malloc(strlen(globals.workdir) +
        strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 10);
    if (NULL == fragment_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(fragment_file, (strlen(globals.workdir) +
        strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 10));
    sprintf(fragment_file, "%s/%s/%s_%d", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, info->get_line());
    OUTPUT_VERBOSE((8, "   %s (%s)", _YELLOW((char *)"extracting it to"),
        fragment_file));

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

int output_function(SgNode *node, fragment_t *fragment) {
    char *function_file = NULL;
    FILE *function_file_FP;

    /* Set fragment filename */
    function_file = (char *)malloc(strlen(globals.workdir) +
        strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 
        strlen(fragment->function_name) + 10);
    if (NULL == function_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(function_file, (strlen(globals.workdir) +
        strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 
        strlen(fragment->function_name) + 10));
    sprintf(function_file, "%s/%s/%s_%s", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, fragment->function_name);
    OUTPUT_VERBOSE((8, "   %s (%s)", _YELLOW((char *)"extracting it to"),
        function_file));

    function_file_FP = fopen(function_file, "w+");
    if (NULL == function_file_FP) {
        OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
            _ERROR(function_file)));
        return PERFEXPERT_ERROR;
    }
    fprintf(function_file_FP, "%s", node->unparseToCompleteString().c_str());
    fclose(function_file_FP);
    free(function_file);

    /* LOG the function */
    LOG((base64_encode(node->unparseToCompleteString().c_str(),
        strlen(node->unparseToCompleteString().c_str()))));

    return PERFEXPERT_SUCCESS;
}

// EOF
