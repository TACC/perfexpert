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

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "aligncheck.h"
#include "analysis_profile.h"
#include "generic_vars.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

aligncheck_t::aligncheck_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
    loop_traversal = new loop_traversal_t(var_renaming);
}

aligncheck_t::~aligncheck_t() {
    delete loop_traversal;
    loop_traversal = NULL;
}

void aligncheck_t::atTraversalStart() {
    statement_list.clear();
}

void aligncheck_t::atTraversalEnd() {
}

bool aligncheck_t::contains_non_linear_reference(
        const reference_list_t& reference_list) {
    for (reference_list_t::const_iterator it2 = reference_list.begin();
            it2 != reference_list.end(); it2++) {
        const reference_info_t& reference_info = *it2;
        const SgNode* ref_node = reference_info.node;

        const SgPntrArrRefExp* pntr = isSgPntrArrRefExp(ref_node);

        // Check if pntr is a linear array reference.
        if (pntr && !ir_methods::is_linear_reference(pntr, false)) {
            return true;
        }
    }

    return false;
}

void aligncheck_t::instrument_streaming_stores(Sg_File_Info* fileInfo,
        loop_info_t& loop_info) {
    // Extract streaming stores from array references in the loop.
    sstore_map_t sstore_map;
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    reference_list_t& reference_list = loop_info.reference_list;

    for (reference_list_t::iterator it2 = reference_list.begin();
            it2 != reference_list.end(); it2++) {
        reference_info_t& reference_info = *it2;
        std::string& ref_stream = reference_info.name;

        switch (reference_info.access_type) {
            case TYPE_WRITE:
                sstore_map[ref_stream].push_back(reference_info.node);
                break;

            case TYPE_READ:
            case TYPE_READ_AND_WRITE:
            case TYPE_UNKNOWN:  // Handle unknown type the same way as a read.
                sstore_map_t::iterator it = sstore_map.find(ref_stream);
                if (it != sstore_map.end()) {
                    sstore_map.erase(it);
                }

                break;
        }
    }

    for (sstore_map_t::iterator it = sstore_map.begin(); it != sstore_map.end();
            it++) {
        const std::string& key = it->first;
        node_list_t& value = it->second;

        // Replace index variable with init expression.
        expr_list_t expr_list;
        for (node_list_t::iterator it = value.begin(); it != value.end();
                it++) {
            if (SgExpression* expr = isSgExpression(*it))
                expr_list.push_back(expr);
        }

        // If we have any expressions, add the instrumentation call.
        if (expr_list.size()) {
            ir_methods::remove_duplicate_expressions(expr_list);
            ir_methods::place_alignment_checks(expr_list, fileInfo, loop_stmt,
                statement_list, "indigo__sstore_aligncheck");
        }
    }
}

#if 0
void aligncheck_t::instrument_pointer_overlap_checks(Sg_File_Info* fileInfo,
        SgScopeStatement* outer_scope_stmt, loop_info_t& loop_info,
        name_list_t& stream_list, expr_map_t& loop_map) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return;
}
#endif

void aligncheck_t::instrument_alignment_checks(Sg_File_Info* fileInfo,
        SgScopeStatement* outer_scope_stmt, loop_info_t& loop_info,
        name_list_t& stream_list, expr_map_t& loop_map) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return;

    SgStatement* first_statement = loop_stmt->firstStatement();
    SgBasicBlock* first_bb = getEnclosingNode<SgBasicBlock>(first_statement);

    expr_list_t expr_list;
    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        if (SgExpression* expr = isSgExpression(reference_info.node))
            expr_list.push_back(expr);
    }

    ir_methods::remove_duplicate_expressions(expr_list);
    ir_methods::place_alignment_checks(expr_list, fileInfo, loop_stmt,
        statement_list, "indigo__aligncheck");
}

void aligncheck_t::process_loop(SgScopeStatement* outer_scope_stmt,
        loop_info_t& loop_info, expr_map_t& loop_map,
        name_list_t& stream_list) {
    int line_number = loop_info.loop_stmt->get_file_info()->get_raw_line();
    loop_map[loop_info.idxv_expr] = &loop_info;

    // Instrument this loop only if
    // the loop header components have been identified.
    // Allow empty init expressions (which is always the case with while and
    // do-while loops).
    if (loop_info.idxv_expr && loop_info.test_expr && loop_info.test_expr
            /* && !contains_non_linear_reference(loop_info.reference_list) */) {
        loop_info.processed = true;

        SgLocatedNode* located_outer_scope =
            reinterpret_cast<SgLocatedNode*>(outer_scope_stmt);
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    located_outer_scope->get_file_info()->get_filenameString());

        instrument_streaming_stores(fileInfo, loop_info);
        instrument_alignment_checks(fileInfo, outer_scope_stmt, loop_info,
                stream_list, loop_map);
    }

    for (std::vector<loop_info_list_t>::iterator it =
            loop_info.child_loop_info.begin();
            it != loop_info.child_loop_info.end(); it++) {
        loop_info_list_t& loop_info_list = *it;
        for (loop_info_list_t::iterator it2 = loop_info_list.begin();
                it2 != loop_info_list.end(); it2++) {
            loop_info_t& loop_info = *it2;
            process_loop(outer_scope_stmt, loop_info, loop_map, stream_list);
        }
    }
}

void aligncheck_t::process_node(SgNode* node) {
    // Begin processing.
    analysis_profile.start_timer();

    // Since this is not really a traversal, manually invoke init function.
    atTraversalStart();

    loop_traversal->traverse(node, attrib());
    loop_info_list_t& loop_info_list = loop_traversal->get_loop_info_list();

    expr_map_t loop_map;
    name_list_t stream_list;

    for (loop_info_list_t::iterator it = loop_info_list.begin();
            it != loop_info_list.end(); it++) {
        loop_info_t& loop_info = *it;
        process_loop(loop_info.loop_stmt, loop_info, loop_map, stream_list);
    }

    // Since this is not really a traversal, manually invoke atTraversalEnd();
    atTraversalEnd();

    analysis_profile.end_timer();
    analysis_profile.set_loop_info_list(loop_info_list);
}

const analysis_profile_t& aligncheck_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator aligncheck_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator aligncheck_t::stmt_end() {
    return statement_list.end();
}
