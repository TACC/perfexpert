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

#ifndef TOOLS_MACPO_INST_INCLUDE_MINST_H_
#define TOOLS_MACPO_INST_INCLUDE_MINST_H_

#include <VariableRenaming.h>

#include <string>

#include "analysis_profile.h"
#include "generic_defs.h"
#include "instrumentor.h"

bool midend(SgProject* project, options_t& options);

class MINST : public AstSimpleProcessing {
 public:
    MINST(const options_t& options, SgProject* project);

    void insert_map_function(SgNode* node);
    void insert_map_prototype(SgNode* node);

    virtual void atTraversalEnd();
    virtual void atTraversalStart();
    virtual void visit(SgNode* node);

 private:
    const options_t& options;
    bool map_function_inserted;

    SgGlobal* global_node;
    VariableRenaming* var_renaming;
    SgFunctionDeclaration *def_decl, *non_def_decl;
    bool is_same_file(const std::string& file_1, const std::string& file_2);

    void analyze_node(SgNode* node, int16_t action);
    const analysis_profile_list run_analysis(SgNode* node, int16_t action);
    void print_loop_processing_status(const loop_info_t& loop_info);
    void add_hooks_to_main_function(SgFunctionDefinition* main_def);

    statement_list_t statement_list;
    name_list_t stream_list;
    name_list_t file_list;
};

#endif  // TOOLS_MACPO_INST_INCLUDE_MINST_H_
