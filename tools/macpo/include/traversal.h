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

#ifndef TOOLS_MACPO_INCLUDE_TRAVERSAL_H_
#define TOOLS_MACPO_INCLUDE_TRAVERSAL_H_

#include <rose.h>

#include <string>

#include "analysis_profile.h"
#include "generic_defs.h"
#include "loop_traversal.h"

class traversal_t {
 public:
    explicit traversal_t(VariableRenaming*& _var_renaming);
    ~traversal_t();

    virtual name_list_t& get_stream_list();
    virtual const analysis_profile_t& get_analysis_profile();
    virtual const statement_list_t::iterator stmt_begin();
    virtual const statement_list_t::iterator stmt_end();
    virtual bool instrument_loop(loop_info_t& loop_info) = 0;

    void process_node(SgNode* node);
    void process_loop(loop_info_t& loop_info);

    void add_stream(std::string stream);
    void add_stmt(statement_info_t& statement_info);
    void set_deep_search(bool _deep_search);
    loop_info_list_t& get_loop_info_list();
    VariableRenaming::NumNodeRenameTable get_defs_at_node(SgNode* node);

 private:
    name_list_t stream_list;
    VariableRenaming* var_renaming;
    statement_list_t statement_list;
    loop_traversal_t* loop_traversal;
    analysis_profile_t analysis_profile;
};

#endif  // TOOLS_MACPO_INCLUDE_TRAVERSAL_H_
