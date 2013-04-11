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

#ifndef TRAVERSAL_H_
#define TRAVERSAL_H_

/* Utility headers */
#ifndef ROSE_H
#include <rose.h>
#endif

#ifndef SAGE3_CLASSES_H
#include <sage3.h>
#endif

#ifndef CI_H_
#include "ci.h"
#endif

class ciTraversal : public AstSimpleProcessing {
    public :

    virtual void visit(SgNode *node);

    function_t *item;
    char *function_name_new;
    SgFunctionSymbol* function_symbol_new;
};

static int insert_function(function_t *function);
static SgFunctionSymbol* build_new_function_declaration(SgStatement* statementLocation,
                                                        SgFunctionType* previousFunctionType,
                                                        char *name);

#endif /* TRAVERSAL_H */