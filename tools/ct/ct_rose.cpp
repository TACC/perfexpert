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

/* PerfExpert headers */
#include "ct_rose.hpp"
#include "perfexpert_alloc.h"
#include "perfexpert_base64.h"
#include "perfexpert_constants.h"
#include "perfexpert_log.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Global variables, try to not create them! */
SgProject *userProject;

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

/* open_rose */
int open_rose(const char *source_file) {
    char *files[3];

    OUTPUT_VERBOSE((7, "      opening Rose"));

    /* Fill 'files', aka **argv */
    PERFEXPERT_ALLOC(char, files[0], (sizeof("recommender") + 1));
    snprintf(files[0], sizeof("recommender"), "recommender");
    PERFEXPERT_ALLOC(char, files[1], (strlen(source_file) + 1));
    strncpy(files[1], source_file, strlen(source_file));
    files[2] = NULL;

    /* Load files and build AST */
    userProject = NULL;
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);
    
    /* I believe now it is OK to free 'argv' */
    PERFEXPERT_DEALLOC(files[0]);
    PERFEXPERT_DEALLOC(files[1]);

    return PERFEXPERT_SUCCESS;
}

/* close_rose */
int close_rose(void) {
    // TODO: should find a way to free 'userProject'
    return PERFEXPERT_SUCCESS;
}

/* extract_fragment */
int extract_fragment(fragment_t *fragment) {
    recommenderTraversal fragmentTraversal;
    char *fragments_dir = NULL;

    /* Create the fragments directory */
    PERFEXPERT_ALLOC(char, fragments_dir,
        (strlen(globals.workdir) + strlen(PERFEXPERT_FRAGMENTS_DIR) + 2));
    sprintf(fragments_dir, "%s/%s", globals.workdir, PERFEXPERT_FRAGMENTS_DIR);
    if (PERFEXPERT_ERROR == perfexpert_util_make_path(fragments_dir, 0755)) {
        OUTPUT(("%s", _ERROR((char *)"Error: cannot create fragments dir")));
        PERFEXPERT_DEALLOC(fragments_dir);
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((4, "      fragments directory (%s)", fragments_dir));
    }
    PERFEXPERT_DEALLOC(fragments_dir);

    /* Build the traversal object and call the traversal function starting at
     * the project node of the AST, using a pre-order traversal
     */
    OUTPUT_VERBOSE((7, "      %s (%s:%d) [code type: %d]",
        _YELLOW((char *)"extracting fragments from"), fragment->filename,
        fragment->line_number, fragment->code_type));

    fragmentTraversal.fragment = fragment;
    fragmentTraversal.traverseInputFiles(userProject, preorder);

    return PERFEXPERT_SUCCESS;
}

/* output_fragment */
int output_fragment(SgNode *node, Sg_File_Info *info,
    fragment_t *fragment) {
    FILE *fragment_file_FP = NULL;
    char *fragment_file = NULL;

    /* Set fragment filename */
    PERFEXPERT_ALLOC(char, fragment_file, (strlen(globals.workdir) +
        strlen(PERFEXPERT_FRAGMENTS_DIR) + strlen(fragment->filename) + 10));
    sprintf(fragment_file, "%s/%s/%s_%d", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, info->get_line());
    OUTPUT_VERBOSE((8, "         %s (%s)", _YELLOW((char *)"extracting it to"),
        fragment_file));

    fragment_file_FP = fopen(fragment_file, "w+");
    if (NULL == fragment_file_FP) {
        OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
            _ERROR(fragment_file)));
        PERFEXPERT_DEALLOC(fragment_file);
        return PERFEXPERT_ERROR;
    }
    fprintf(fragment_file_FP, "%s", node->unparseToCompleteString().c_str());
    fclose(fragment_file_FP);
    PERFEXPERT_DEALLOC(fragment_file);

    return PERFEXPERT_SUCCESS;
}

/* output_function */
int output_function(SgNode *node, fragment_t *fragment) {
    FILE *function_file_FP = NULL;
    char *function_file = NULL;

    /* Set fragment filename */
    PERFEXPERT_ALLOC(char, function_file,
        (strlen(globals.workdir) + strlen(PERFEXPERT_FRAGMENTS_DIR) +
            strlen(fragment->filename) + strlen(fragment->function_name) + 10));
    sprintf(function_file, "%s/%s/%s_%s", globals.workdir,
        PERFEXPERT_FRAGMENTS_DIR, fragment->filename, fragment->function_name);
    OUTPUT_VERBOSE((8, "         %s (%s)", _YELLOW((char *)"extracting it to"),
        function_file));

    function_file_FP = fopen(function_file, "w+");
    if (NULL == function_file_FP) {
        OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
            _ERROR(function_file)));
        PERFEXPERT_DEALLOC(function_file);
        return PERFEXPERT_ERROR;
    }
    fprintf(function_file_FP, "%s", node->unparseToCompleteString().c_str());
    fclose(function_file_FP);
    PERFEXPERT_DEALLOC(function_file);

    /* LOG the function */
    LOG((base64_encode(node->unparseToCompleteString().c_str(),
        strlen(node->unparseToCompleteString().c_str()))));

    return PERFEXPERT_SUCCESS;
}

// EOF
