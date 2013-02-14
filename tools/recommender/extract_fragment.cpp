/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of OptTran and PerfExpert.
 *
 * OptTran as well PerfExpert are free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * OptTran and PerfExpert are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
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
#ifndef ROSE_H
#include <rose.h>
#endif

#ifndef SAGE3_CLASSES_H
#include <sage3.h>
#endif

/* OptTran headers */
#include "config.h"
#include "recommender.h"
#include "opttran_output.h"
#include "opttran_util.h"

extern globals_t globals; // globals was defined on 'recommender.c'

class visitorTraversal : public AstSimpleProcessing {
    public :
    virtual void visit(SgNode* n);
    virtual void atTraversalStart();
    virtual void atTraversalEnd();
};

void visitorTraversal::visit(SgNode* n) {
    if ((NULL != isSgForStatement(n)) || (NULL != isSgFortranDo(n))) {
        Sg_File_Info &fileInfo = *(n->get_file_info());
        // static SgNode* lastFor = NULL;

//        if (fileInfo.get_line()) {
//            
//        }
        printf("Found a %s on '%s:%d', extracing it:\n", n->sage_class_name(),
               fileInfo.get_filename(), fileInfo.get_line());
        printf("%s\n", n->unparseToCompleteString().c_str());
        
//        printf("Found a for loop: hooking a printf here...\n");
//        SgExprStatement * printIt =
//        SageBuilder::buildFunctionCallStmt("printf",
//                                           SageBuilder::buildVoidType(),
//                                           SageBuilder::buildExprListExp(isSgExpression(n)),
//                                           SageInterface::getScope(n));
//        SageInterface::insertStatementAfter(isSgStatement(n), printIt);
//        SgSourceFile* file = isSgSourceFile(buildFile("teste.c", "teste.c", project));
//        SgNode* test = deepCopyNode(n);
//        deleteAST (SgNode *node)
    }
}

void visitorTraversal::atTraversalStart() {
    OPTTRAN_OUTPUT_VERBOSE((9, "%s (%s)",
                            _YELLOW((char *)"starting traversal on "),
                            segment->filename));
}

void visitorTraversal::atTraversalEnd() {
    OPTTRAN_OUTPUT_VERBOSE((9, "%s (%s)",
                            _YELLOW((char *)"ending traversal on "),
                            segment->filename));
}

int extract_fragment(segment_t *segment) {
    char **files;
    int filenum;
    int i;

    OPTTRAN_OUTPUT_VERBOSE((7, "%s (%s:%d)",
                            _GREEN((char *)"extracting fragment from"),
                            segment->filename, segment->line_number));
    
    /* Fill 'files', aka **argv */
    files = (char **)malloc(sizeof(char *) * 3);
    files[0] = (char *)malloc(sizeof("recommender") + 1);
    if (NULL == files[0]) {
        OPTTRAN_OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    bzero(files[0], sizeof("recommender") + 1);
    snprintf(files[0], sizeof("recommender"), "recommender");
    files[1] = globals.source_file;
    files[2] = NULL;

    /* Build the AST */
    SgProject* project = frontend(2, files);
    ROSE_ASSERT(project != NULL);

    /* Build the traversal object and call the traversal function
     * starting at the project node of the AST, using a pre-order traversal
     */
    visitorTraversal exampleTraversal;
    exampleTraversal.traverseInputFiles(project, preorder);

    /* Insert manipulations of the AST here... */

    /* Generate source code output on OPTTRAN directory */
    // TODO: generate files in the right place
    filenum = project->numberOfFiles();
    for (i = 0; i < filenum; ++i) {
        SgSourceFile* file = isSgSourceFile(project->get_fileList()[i]);
        file->unparse();
    }

    free(files[0]);
    free(files);

    OPTTRAN_OUTPUT_VERBOSE((7, "%s",
                            _GREEN((char *)"fragment extraction concluded")));
    
    return OPTTRAN_SUCCESS;
}

// EOF
