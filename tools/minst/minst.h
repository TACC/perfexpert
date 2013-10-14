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

#ifndef	MINST_H_
#define	MINST_H_

#include "generic_defs.h"
#include "instrumentor.h"

class MINST : public AstTopDownProcessing<attrib> {
    short action;
    int line_number;
    std::string inst_func;
    std::vector<reference_info_t> reference_list;

    SgGlobal* global_node;
    Sg_File_Info* file_info;
    SgFunctionDeclaration *def_decl, *non_def_decl;

    public:
    MINST(short _action, int _line_number, std::string _inst_func);

    void insert_map_function(SgNode* node);
    void insert_map_prototype(SgNode* node);

    virtual void atTraversalEnd();
    virtual void atTraversalStart();

    virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* MINST_H_ */
