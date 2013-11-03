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

#ifndef CT_ROSE_H_
#define CT_ROSE_H_

/* Utility headers */
#ifndef ROSE_H
#include <rose.h>
#endif

#ifndef SAGE3_CLASSES_H
#include <sage3.h>
#endif

/* PerfExpert tool headers */
#include "ct.h"

extern SgProject *userProject; /* This variable is declared in ct_rose.cpp */

class recommenderTraversal : public AstSimpleProcessing {
    public :
        virtual void visit(SgNode *node);
        virtual void atTraversalStart();
        virtual void atTraversalEnd();
        fragment_t *fragment;
        int rc;
};

/* Function declarations */
int output_fragment(SgNode *node, Sg_File_Info *fileInfo, fragment_t *fragment);
int output_function(SgNode *node, fragment_t *fragment);

#endif /* CT_ROSE_H */
