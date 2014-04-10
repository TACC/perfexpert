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

#include <rose.h>

#include <string>
#include <vector>

#include "traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

traversal_t::traversal_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
    loop_traversal = new loop_traversal_t(var_renaming);
}

traversal_t::~traversal_t() {
    delete loop_traversal;
    loop_traversal = NULL;
}

void traversal_t::process_loop(loop_info_t& loop_info) {
    // Instrument this loop only if
    // the loop header components have been identified.
    // Allow empty init expressions (which is always the case with while and
    // do-while loops).
    if (loop_info.idxv_expr && loop_info.test_expr && loop_info.test_expr) {
        loop_info.processed = true;
        instrument_loop(loop_info);
    }

    std::vector<loop_info_list_t>& child_loop_info = loop_info.child_loop_info;
    for (std::vector<loop_info_list_t>::iterator it = child_loop_info.begin();
            it != child_loop_info.end(); it++) {
        loop_info_list_t& loop_info_list = *it;
        for (loop_info_list_t::iterator it2 = loop_info_list.begin();
                it2 != loop_info_list.end(); it2++) {
            loop_info_t& loop_info = *it2;
            process_loop(loop_info);
        }
    }
}

void traversal_t::process_node(SgNode* node) {
    // Start profiling.
    analysis_profile.start_timer();

    stream_list.clear();
    statement_list.clear();

    // Traverse all loops within this node.
    loop_traversal->traverse(node, attrib());

    loop_info_list_t& loop_info_list = get_loop_info_list();
    analysis_profile.set_loop_info_list(loop_info_list);

    for (loop_info_list_t::iterator it = loop_info_list.begin();
            it != loop_info_list.end(); it++) {
        loop_info_t& loop_info = *it;
        process_loop(loop_info);
    }

    // End profiling.
    analysis_profile.end_timer();
}

name_list_t& traversal_t::get_stream_list() {
    return stream_list;
}

const analysis_profile_t& traversal_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator traversal_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator traversal_t::stmt_end() {
    return statement_list.end();
}

void traversal_t::add_stmt(statement_info_t& statement_info) {
    statement_list.push_back(statement_info);
}

void traversal_t::add_stream(std::string stream) {
    stream_list.push_back(stream);
}

loop_info_list_t& traversal_t::get_loop_info_list() {
    return loop_traversal->get_loop_info_list();
}

VariableRenaming::NumNodeRenameTable traversal_t::get_defs_at_node(SgNode*
        node) {
    return var_renaming->getReachingDefsAtNode(node);
}
