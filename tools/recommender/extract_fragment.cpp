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
        // static SgNode* lastFor = NULL;
        Sg_File_Info &fileInfo = *(n->get_file_info());
        
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
    printf("Traversal starts here.\n");
}

void visitorTraversal::atTraversalEnd() {
    printf("Traversal ends here.\n");
}

int extract_fragment(segment_t *segment) {
    char **files;

    OPTTRAN_OUTPUT_VERBOSE((7, "%s (%s:%d)",
                            _GREEN((char *)"extracting fragment for"),
                            segment->filename, segment->line_number));
    
    /* Fill 'files', aka **argv */
    printf("ERRO 1.0\n");
    **files = malloc(sizeof(char*) * 3);

    printf("ERRO 1.1\n");
    files[0] = (char *)malloc(sizeof("recommender") + 1);
    printf("ERRO 1.2\n");
    if (NULL == files[0]) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    printf("ERRO 1.3\n");
    bzero(files[0], sizeof("recommender") + 1);
    printf("ERRO 1.4\n");
    sprintf(files[0], sizeof("recommender"), "recommender");
    
    printf("ERRO 1.5\n");
    files[1] = &(globals.source_file);
    printf("ERRO 1.6\n");
    files[2] = NULL;
    printf("ERRO 1.7\n");

    /* Build the AST */
    printf("ERRO 1\n");
    SgProject* project = frontend(1, files);
    printf("ERRO 2\n");
    ROSE_ASSERT(project != NULL);
    printf("ERRO 3\n");

    /* Build the traversal object and call the traversal function
     * starting at the project node of the AST, using a pre-order traversal
     */
    visitorTraversal exampleTraversal;
    printf("ERRO 4\n");
    exampleTraversal.traverseInputFiles(project, preorder);

    /* Insert manipulations of the AST here... */

    /* Generate source code output */
    printf("ERRO 5\n");
    int filenum = project->numberOfFiles();
    printf("ERRO 6\n");
    for (int i=0; i<filenum; ++i) {
        printf("ERRO 7\n");
        SgSourceFile* file = isSgSourceFile(project->get_fileList()[i]);
        printf("ERRO 8\n");
        file->unparse();
        printf("ERRO 9\n");
    }
    printf("ERRO 10\n");

    return OPTTRAN_SUCCESS;
}

// EOF
