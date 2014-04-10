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

#include <string>

#include "aligncheck.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

bool aligncheck_t::instrument_streaming_stores(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_info.loop_stmt->get_file_info();

    // Extract streaming stores from array references in the loop.
    sstore_map_t sstore_map;
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
            statement_list_t _stmt_list;
            ir_methods::remove_duplicate_expressions(expr_list);
            ir_methods::place_alignment_checks(expr_list, fileInfo, loop_stmt,
                    _stmt_list, "indigo__sstore_aligncheck");

            for (statement_list_t::iterator it = _stmt_list.begin();
                    it != _stmt_list.end(); it++) {
                statement_info_t& stmt = *it;
                add_stmt(stmt);
            }
        }
    }

    return true;
}

bool aligncheck_t::instrument_alignment_checks(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_info.loop_stmt->get_file_info();

    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return true;

    SgStatement* first_statement = loop_stmt->firstStatement();
    SgBasicBlock* first_bb = getEnclosingNode<SgBasicBlock>(first_statement);

    expr_list_t expr_list;
    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        if (SgExpression* expr = isSgExpression(reference_info.node))
            expr_list.push_back(expr);
    }

    statement_list_t _stmt_list;
    ir_methods::remove_duplicate_expressions(expr_list);
    ir_methods::place_alignment_checks(expr_list, fileInfo, loop_stmt,
        _stmt_list, "indigo__aligncheck");

    for (statement_list_t::iterator it = _stmt_list.begin();
            it != _stmt_list.end(); it++) {
        statement_info_t& stmt = *it;
        add_stmt(stmt);
    }

    return true;
}

bool aligncheck_t::instrument_loop(loop_info_t& loop_info) {
    return instrument_alignment_checks(loop_info) &&
        instrument_streaming_stores(loop_info);
}
