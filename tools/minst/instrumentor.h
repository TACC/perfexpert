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

#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

typedef struct {
    SgBasicBlock* bb;
    SgExprStatement* exprStmt;
    std::vector<SgExpression*> params;
} inst_info_t;

class instrumentor_t : public AstTopDownProcessing<attrib> {
    private:
        std::vector<std::string> stream_list;
        std::vector<inst_info_t> inst_info_list;

    public:
        std::vector<std::string>& get_stream_list();

        virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
        virtual void atTraversalStart();
        virtual void atTraversalEnd();
};

#endif	/* INSTRUMENTOR_H_ */
