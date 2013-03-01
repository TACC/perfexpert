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

#define OPTTRAN_LOOP_LABEL 16

class recommenderTraversal : public AstSimpleProcessing {
    public :
    virtual void visit(SgNode *node);
    virtual void atTraversalStart();
    virtual void atTraversalEnd();
    segment_t *item;
};

static int output_fragment(SgNode *node, Sg_File_Info *fileInfo,
	                       segment_t *item);

#endif /* TRAVERSAL_H */

// EOF
