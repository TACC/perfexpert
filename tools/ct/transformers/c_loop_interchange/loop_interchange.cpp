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
#include <unistd.h>
#include <string.h>
#include <getopt.h> /* To parse command line arguments */

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/** Structure to hold global variables */
typedef struct {
    char *workdir;
    char *sourcefile;
    char *resultfile;
    char *function;
    int  linenumber;
} globals_t;

globals_t  globals;

/* PerfExpert headers */
#include "config.h"
#include "perfexpert_constants.h"

using namespace std;
using namespace SageInterface;

SgProject *userProject;

class loopTraversal : public AstSimpleProcessing {
    public :
    virtual void visit(SgNode *node);
    int rc;
};

static int parse_cli_params(int argc, char *argv[]);
int open_rose(void);
int extract_source(void);

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"linenumber",      required_argument, NULL, 'l'},
    {"function",        required_argument, NULL, 'f'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"resultfile",      required_argument, NULL, 'r'},
    {"sourcefile",      required_argument, NULL, 's'},
    {"workdir",         required_argument, NULL, 'w'},
    {0, 0, 0, 0}
};

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code
int main(int argc, char** argv) {
    loopTraversal segmentTraversal;

    /* Set default values for globals */
    globals.workdir     = NULL; // char *
    globals.sourcefile  = NULL; // char *
    globals.resultfile  = NULL; // char *
    globals.function    = NULL; // char *
    globals.linenumber  = 0;    // int

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        printf("Error: parsing command line arguments\n");
        return PERFEXPERT_ERROR;
    }
    
    /* Open ROSE */
    if (PERFEXPERT_ERROR == open_rose()) {
        printf("Error: starting Rose, exiting...\n");
        return PERFEXPERT_ERROR;
    }

    segmentTraversal.rc = false;
    segmentTraversal.traverseInputFiles(userProject, preorder);

    /* Output source code */
    if (PERFEXPERT_SUCCESS != extract_source()) {
        printf("Error: extracting source code\n");
        return PERFEXPERT_ERROR;
    }

    if (false == segmentTraversal.rc) {
        return PERFEXPERT_ERROR;
    } else {
        return PERFEXPERT_SUCCESS;
    }
}

static int parse_cli_params(int argc, char *argv[]) {
    int parameter; /** Temporary variable to hold parameter */
    int option_index = 0; /** getopt_long() stores the option index here */
    int rc = PERFEXPERT_ERROR;
    
    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "l:f:o:r:s:w:",
                                long_options, &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            case 'l':
                globals.linenumber = atoi(optarg);
                if (0 >= atoi(optarg)) {
                    printf("Error: invalid line number (%d)", atoi(optarg));
                }
                break;

            case 'f':
                globals.function = optarg;
                break;

            case 'r':
                globals.resultfile = optarg;
                break;

            case 's':
                globals.sourcefile = optarg;
                break;

            case 'w':
                globals.workdir = optarg;
                break;

            default:
                exit(PERFEXPERT_ERROR);
        }
    }
    printf("Summary of selected options:\n");
    printf("   Line number:   %d\n", globals.linenumber);
    printf("   Function name: %s\n", globals.function ? globals.function : "(null)");
    printf("   Result file:   %s\n", globals.resultfile ? globals.resultfile : "(null)");
    printf("   Source file:   %s\n", globals.sourcefile ? globals.sourcefile : "(null)");
    printf("   Workdir:       %s\n", globals.workdir ? globals.workdir : "(null)");

    /* Basic checks */
    if (NULL == globals.sourcefile) {
        printf("Error: source file name missing\n");
    } else if (NULL == globals.function) {
        printf("Error: function name missing\n");
    } else if (NULL == globals.resultfile) {
        printf("Error: result file name missing\n");
    } else if (0 == globals.linenumber) {
        printf("Error: line number missing\n");
    } else {
       rc = PERFEXPERT_SUCCESS;
    }
    return rc;
}

int open_rose(void) {
    char **files = NULL;

    /* Fill 'files', aka **argv */
    files = (char **)malloc(sizeof(char *) * 3);
    files[0] = (char *)malloc(sizeof("ct_loop_interchange") + 1);
    if (NULL == files[0]) {
        printf("Error: out of memory\n");
        return PERFEXPERT_ERROR;
    }
    bzero(files[0], sizeof("ct_loop_interchange") + 1);
    snprintf(files[0], sizeof("ct_loop_interchange"), "ct_loop_interchange");

    files[1] = globals.sourcefile;
    files[2] = NULL;

    /* Load files and build AST */
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);
    
    return PERFEXPERT_SUCCESS;
}

int extract_source(void) {
    FILE *destination_file_FP;
    SgSourceFile* file = NULL;
    int fileNum;
    int i;

    /* For each filename */
    fileNum = userProject->numberOfFiles();
    for (i = 0; i < fileNum; ++i) {
        file = isSgSourceFile(userProject->get_fileList()[i]);

        /* Open output file */
        destination_file_FP = fopen(globals.resultfile, "w+");
        if (NULL == destination_file_FP) {
            printf("error opening file (%s)\n", globals.resultfile);
            return PERFEXPERT_ERROR;
        }

        /* Output source code */
        fprintf(destination_file_FP, "%s",
                file->unparseToCompleteString().c_str());
        
        /* Close output file */
        fclose(destination_file_FP);

        printf("modified source code (%s)\n", globals.resultfile);
    }
    return PERFEXPERT_SUCCESS;
}

void loopTraversal::visit(SgNode *node) {
    Sg_File_Info *info = NULL;
    SgNode *parent = NULL;

    info = node->get_file_info();

    /* Find code fragment for bottlenecks type 'loop' in C */
    if ((isSgForStatement(node)) &&
        (info->get_line() == globals.linenumber)) {

        /* Found a C loop on the exact line number */
        printf("found a loop on (%s:%d) looking for the parent loop\n",
               info->get_filename(), info->get_line());

        parent = node->get_parent();
        info = parent->get_file_info();

        /* It is a basic block. Who is this basic block's parent? */
        if (NULL != isSgBasicBlock(parent)) {
            parent = parent->get_parent();
        }

        /* Is it a for/do/while? */
        if (isSgForStatement(parent)) {

            /* The parent is a loop */
            printf("parent loop found on (%s:%d)\n", info->get_filename(),
                   info->get_line());

            /* Do the interchange */
            rc = loopInterchange(isSgForStatement(parent), 2, 1);
        }
    }
}

// EOF
