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
#include <getopt.h> /* To parse command line arguments */

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/** Structure to hold global variables */
typedef struct {
    char *inputfile;
    char *outputfile;
    char *workdir;
    char *sourcefile;
    char *resultfile;
    char *function;
    int  linenumber;
    int  colorful; // not used in this program
    int  verbose;
    int  verbose_level;
} globals_t;

globals_t  globals;

/* WARNING: to include perfexpert_output.h globals have to be defined first */
#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[ct_loop_interchange]"

/* PerfExpert headers */
#include "config.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;

SgProject *userProject;

class loopTraversal : public AstSimpleProcessing {
    public :
    virtual void visit(SgNode *node);
    virtual void atTraversalStart();
    virtual void atTraversalEnd();
};

static int parse_cli_params(int argc, char *argv[]);
int open_rose(void);
int extract_source(void);

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"linenumber",      required_argument, NULL, 'l'},
    {"debug",           no_argument,       NULL, 'd'},
    {"function",        required_argument, NULL, 'f'},
    {"outputfile",      required_argument, NULL, 'o'},
    {"resultfile",      required_argument, NULL, 'r'},
    {"sourcefile",      required_argument, NULL, 's'},
    {"workdir",         required_argument, NULL, 'w'},
    {"not_used",        no_argument,       NULL, 'c'},
    {"not_used",        no_argument,       NULL, 'p'},
    {"help",            no_argument,       NULL, 'h'},
    {0, 0, 0, 0}
};

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

int main(int argc, char** argv) {
    loopTraversal segmentTraversal;

    /* Set default values for globals */
    globals.verbose     = 0;    // int
    globals.inputfile   = NULL; // char *
    globals.outputfile  = NULL; // char *
    globals.workdir     = NULL; // char *
    globals.sourcefile  = NULL; // char *
    globals.function    = NULL; // char *
    globals.linenumber  = 0;    // int

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR((char *)"Error: parsing command line arguments")));
        exit(PERFEXPERT_ERROR);
    }
    
    /* Open ROSE */
    if (PERFEXPERT_ERROR == open_rose()) {
        OUTPUT(("%s", _ERROR((char *)"Error: starting Rose, exiting...")));
        exit(PERFEXPERT_ERROR);
    } else {
        OUTPUT(("Opening ROSE..."));
    }

    /* Build the traversal object and call the traversal function starting at
     * the project node of the AST, using a pre-order traversal
     */
    OUTPUT(("Calling traversal..."));
    segmentTraversal.traverseInputFiles(userProject, preorder);

    /* Output source code */
    if (PERFEXPERT_SUCCESS != extract_source()) {
        OUTPUT(("%s", _ERROR((char *)"Error: extracting source code")));
        exit(PERFEXPERT_ERROR);
    } else {
        OUTPUT(("Extracting source..."));
    }

    OUTPUT(("Closing ROSE..."));

    return PERFEXPERT_SUCCESS;
}

static int parse_cli_params(int argc, char *argv[]) {
    /** Temporary variable to hold parameter */
    int parameter;
    /** getopt_long() stores the option index here */
    int option_index = 0;

    while (1) {
        /* get parameter */
        parameter = getopt_long(argc, argv, "l:df:o:r:s:w:cph",
                                long_options, &option_index);

        /* Detect the end of the options */
        if (-1 == parameter) {
            break;
        }

        switch (parameter) {
            case 'l':
                globals.linenumber = atoi(optarg);
                OUTPUT_VERBOSE((10, "option 'l' set"));
                if (0 >= atoi(optarg)) {
                    OUTPUT(("%s (%d)",
                            _ERROR((char *)"Error: invalid line number"),
                            atoi(optarg)));
                }
                break;

            case 'd':
            case 'c':
            case 'p':
                OUTPUT_VERBOSE((10, "option 'c' set (not used)"));
                break;

            case 'h':
                OUTPUT_VERBOSE((10, "option 'h' set (help not implemented)"));

            case 'f':
                globals.function = optarg;
                OUTPUT_VERBOSE((10, "option 'f' set [%s]", globals.function));
                break;

            case 'o':
                globals.outputfile = optarg;
                OUTPUT_VERBOSE((10, "option 'o' set [%s]", globals.outputfile));
                break;

            case 'r':
                globals.resultfile = optarg;
                OUTPUT_VERBOSE((10, "option 'r' set [%s]", globals.resultfile));
                break;

            case 's':
                globals.sourcefile = optarg;
                OUTPUT_VERBOSE((10, "option 's' set [%s]", globals.sourcefile));
                break;

            case 'w':
                globals.workdir = optarg;
                OUTPUT_VERBOSE((10, "option 'w' set [%s]", globals.workdir));
                break;

            default:
                exit(PERFEXPERT_ERROR);
        }
    }
    OUTPUT(("Summary of selected options:"));
    OUTPUT(("   Line number:   %d", globals.linenumber));
    OUTPUT(("   Function name: %s", globals.function ? globals.function : "(null)"));
    OUTPUT(("   Output file:   %s", globals.outputfile ? globals.outputfile : "(null)"));
    OUTPUT(("   Result file:   %s", globals.resultfile ? globals.resultfile : "(null)"));
    OUTPUT(("   Source file:   %s", globals.sourcefile ? globals.sourcefile : "(null)"));
    OUTPUT(("   Workdir:       %s", globals.workdir ? globals.workdir : "(null)"));

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose_level) {
        int i;
        printf("%s complete command line:", PROGRAM_PREFIX);
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* Basic checks */
    if (NULL == globals.sourcefile) {
        OUTPUT(("%s", _ERROR((char *)"Error: source file name missing")));
        exit(PERFEXPERT_ERROR);
    }
    if (NULL == globals.function) {
        OUTPUT(("%s", _ERROR((char *)"Error: function name missing")));
        exit(PERFEXPERT_ERROR);
    }
    if (NULL == globals.resultfile) {
        OUTPUT(("%s", _ERROR((char *)"Error: result file name missing")));
        exit(PERFEXPERT_ERROR);
    }
    if (0 == globals.linenumber) {
        OUTPUT(("%s", _ERROR((char *)"Error: line number missing")));
        exit(PERFEXPERT_ERROR);
    }
    return PERFEXPERT_SUCCESS;
}

int open_rose(void) {
    char **files = NULL;

    /* Fill 'files', aka **argv */
    files = (char **)malloc(sizeof(char *) * 3);
    files[0] = (char *)malloc(sizeof("ct_loop_interchange") + 1);
    if (NULL == files[0]) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(files[0], sizeof("ct_loop_interchange") + 1);
    snprintf(files[0], sizeof("ct_loop_interchange"), "ct_loop_interchange");

    files[1] = (char *)malloc(sizeof(globals.sourcefile) +
                              sizeof(globals.workdir) + 2);
    if (NULL == files[1]) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(files[1], sizeof(globals.sourcefile) + sizeof(globals.workdir) + 2);
    sprintf(files[1], "%s/%s", globals.workdir, globals.sourcefile);

    files[2] = NULL;

    /* Load files and build AST */
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);
    
    /* I believe now it is OK to free 'argv' */
    // free(files[0]);
    // free(files[1]);
    // free(files);

    return PERFEXPERT_SUCCESS;
}

int extract_source(void) {
    FILE *destination_file_FP;
    char *destination_file = NULL;
    SgSourceFile* file = NULL;
    int fileNum;
    int i;

    destination_file = (char *)malloc(sizeof(globals.resultfile) +
                                      sizeof(globals.workdir) + 3);
    if (NULL == destination_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(destination_file, sizeof(globals.resultfile) + sizeof(globals.workdir) + 3);
    sprintf(destination_file, "%s/%s", globals.workdir, globals.resultfile);

    /* For each filename */
    fileNum = userProject->numberOfFiles();
    for (i = 0; i < fileNum; ++i) {
        file = isSgSourceFile(userProject->get_fileList()[i]);

        /* Open output file */
        destination_file_FP = fopen(destination_file, "w+");
        if (NULL == destination_file_FP) {
            OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
                    _ERROR(destination_file)));
            return PERFEXPERT_ERROR;
        }

        /* Output source code */
        fprintf(destination_file_FP, "%s",
                file->unparseToCompleteString().c_str());
        
        /* Close output file */
        fclose(destination_file_FP);

        OUTPUT(("modified source code (%s)", destination_file));
        // free(destination_file);
    }
    return PERFEXPERT_SUCCESS;
}

void loopTraversal::visit(SgNode *node) {
    Sg_File_Info *fileInfo = NULL;
    SgForStatement *c_loop = NULL;
    SgFortranDo *f_loop = NULL;
    int done = 0;

    fileInfo = node->get_file_info();

    /* Find code fragment for bottlenecks type 'loop' in C */
    if ((NULL != (c_loop = isSgForStatement(node))) &&
        (fileInfo->get_line() == globals.linenumber)) {

        /* Found a C loop on the exact line number */
        OUTPUT(("found a (%s) on (%s:%d)", node->sage_class_name(),
                fileInfo->get_filename(), fileInfo->get_line()));

        loopInterchange(isSgForStatement(node), 2, 1);
        done = 1;
    }
}

void loopTraversal::atTraversalStart() {
    OUTPUT(("%s (%s)", _YELLOW((char *)"starting traversal on"),
            globals.sourcefile));
}

void loopTraversal::atTraversalEnd() {
    OUTPUT(("%s (%s)", _YELLOW((char *)"ending traversal on"),
            globals.sourcefile));
}

// EOF
