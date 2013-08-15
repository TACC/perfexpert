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

#ifndef TRAVERSAL_H_
#define TRAVERSAL_H_

/* Utility headers */
#ifndef ROSE_H
#include <rose.h>
#endif

#ifndef SAGE3_CLASSES_H
#include <sage3.h>
#endif

#ifndef CT_H_
#include "ct.h"
#endif

class recommenderTraversal : public AstSimpleProcessing {
    public :
    virtual void visit(SgNode *node);
    virtual void atTraversalStart();
    virtual void atTraversalEnd();
    fragment_t *fragment;
    int rc;
};

static int output_fragment(SgNode *node, Sg_File_Info *fileInfo,
	                       fragment_t *fragment);

static int output_function(SgNode *node, fragment_t *fragment);

#endif /* TRAVERSAL_H */
