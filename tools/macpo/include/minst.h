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

#include <VariableRenaming.h>

#include "generic_defs.h"
#include "instrumentor.h"

class MINST : public AstSimpleProcessing {
    public:
        MINST(short _action, int _line_number, std::string _inst_func, VariableRenaming* _var_renaming);

        void insert_map_function(SgNode* node);
        void insert_map_prototype(SgNode* node);

        virtual void atTraversalEnd();
        virtual void atTraversalStart();
        virtual void visit(SgNode* node);

    private:
        short action;
        int line_number;
        std::string inst_func;

        SgGlobal* global_node;
        Sg_File_Info* file_info;
        VariableRenaming* var_renaming;
        SgFunctionDeclaration *def_decl, *non_def_decl;

        inst_list_t inst_info_list;
        name_list_t stream_list;
};

#endif	/* MINST_H_ */
