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
    OPTTRAN_OUTPUT_VERBOSE((7, "%s (%s:%d)",
                            _GREEN((char *)"extracting fragment for"),
                            segment->filename, segment->line_number));
    
    /* Build the AST */
    SgProject* project = frontend(0, NULL);
    ROSE_ASSERT(project != NULL);

    /* Build the traversal object and call the traversal function
     * starting at the project node of the AST, using a pre-order traversal
     */
    visitorTraversal exampleTraversal;
    exampleTraversal.traverseInputFiles(project, preorder);

    /* Insert manipulations of the AST here... */

    /* Generate source code output */
    int filenum = project->numberOfFiles();
    for (int i=0; i<filenum; ++i) {
        SgSourceFile* file = isSgSourceFile(project->get_fileList()[i]);
        file->unparse();
    }

    return OPTTRAN_SUCCESS;
}

// EOF
