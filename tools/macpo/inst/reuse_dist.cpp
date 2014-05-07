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

#include <set>
#include <string>
#include <vector>

#include "reuse_dist.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

bool reuse_dist_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();
    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return true;

    SgBasicBlock* bb = getEnclosingNode<SgBasicBlock>(loop_stmt);
    std::string function_name = SageInterface::is_Fortran_language()
        ? "indigo__reuse_dist_f" : "indigo__reuse_dist_c";

    // Collect indexes of refs that are read or read & written.
    std::set<int64_t> read_ref_idx;
    for (reference_list_t::iterator it = reference_list.begin();
        it != reference_list.end(); it++) {
        reference_info_t& ref = *it;
        if (ref.access_type != TYPE_WRITE) {
            read_ref_idx.insert(ref.idx);
        }
    }

    // Use the write-only references and discard the rest.
    reference_list_t unique_writeonly_refs;
    for (reference_list_t::iterator it = reference_list.begin();
        it != reference_list.end(); it++) {
        reference_info_t& ref = *it;
        if (read_ref_idx.find(ref.idx) == read_ref_idx.end()) {
            unique_writeonly_refs.push_back(ref);
        }
    }

    for (reference_list_t::iterator it = unique_writeonly_refs.begin();
        it != unique_writeonly_refs.end(); it++) {
        reference_info_t& ref = *it;

        // Add all references to the stream list,
        // skipping any will mess up the indexing.
        add_stream(ref.name);

        // If the reference is not an expression or if it not a write-only expr,
        // then don't process it.
        SgExpression* ref_expr = reinterpret_cast<SgExpression*>(ref.node);
        if (ref_expr == NULL || ref.access_type != TYPE_WRITE) {
            continue;
        }

        SgStatement* stmt = NULL;
        if (SgStatement* _stmt = isSgStatement(ref.node)) {
            stmt = _stmt;
        } else {
            stmt = getEnclosingNode<SgStatement>(ref.node);
        }

        // This instrumentation call takes two parameters: index and address.
        SgIntVal* param_idx = new SgIntVal(fileInfo, ref.idx);
        param_idx->set_endOfConstruct(fileInfo);

        SgExpression *param_addr = SageInterface::is_Fortran_language() ?
            ref_expr : buildCastExp(buildAddressOfOp(ref_expr),
                    buildPointerType(buildVoidType()));
        param_addr->set_endOfConstruct(fileInfo);

        std::vector<SgExpression*> params;
        params.push_back(param_idx);
        params.push_back(param_addr);

        // Add the instrumentation call.
        statement_info_t reuse_dist_stmt;
        reuse_dist_stmt.statement = ir_methods::prepare_call_statement(bb,
                function_name, params, loop_stmt);
        reuse_dist_stmt.reference_statement = stmt;
        reuse_dist_stmt.before = false;
        add_stmt(reuse_dist_stmt);
    }

    return true;
}
