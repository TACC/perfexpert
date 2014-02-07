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

#include <algorithm>
#include <rose.h>

#include "analysis_profile.h"
#include "vector_strides.h"
#include "generic_vars.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

vector_strides_t::vector_strides_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
}

name_list_t& vector_strides_t::get_stream_list() {
    return var_name_list;
}

void vector_strides_t::atTraversalStart() {
    analysis_profile.start_timer();
    var_name_list.clear();
    statement_list.clear();
}

void vector_strides_t::atTraversalEnd() {
    analysis_profile.end_timer();
}

bool vector_strides_t::contains_non_linear_reference(
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

void vector_strides_t::instrument_vector_strides(Sg_File_Info* fileInfo,
        SgScopeStatement* scope_stmt) {
    generic_vars_t generic_vars(true);
    generic_vars.traverse(scope_stmt, attrib());
    reference_list_t& var_list = generic_vars.get_reference_list();

    size_t count = 0;
    for(reference_list_t::iterator it = var_list.begin(); it != var_list.end();
            it++) {
        reference_info_t& reference_info = *it;
        std::string stream = reference_info.name;
        SgNode* ref_node = reference_info.node;

        if (count == reference_info.idx) {
            var_name_list.push_back(stream);
            count += 1;
        }

        SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(ref_node);
        SgStatement* containingStmt = getEnclosingNode<SgStatement>(ref_node);

        int line_number = 0;
        SgStatement *stmt = getEnclosingNode<SgStatement>(ref_node);
        if (stmt)	line_number = stmt->get_file_info()->get_raw_line();

        if (line_number == 0)
            continue;

        line_number = scope_stmt->get_file_info()->get_raw_line();

        SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
        SgIntVal* param_idx = new SgIntVal(fileInfo, reference_info.idx);

        // If not Fortran, cast the address to a void pointer
        SgExpression *param_addr = SageInterface::is_Fortran_language() ?
            (SgExpression*) ref_node : buildCastExp (
                    buildAddressOfOp((SgExpression*) ref_node),
                    buildPointerType(buildVoidType()));

        std::string function_name = SageInterface::is_Fortran_language() ?
                "indigo__vector_stride_f" : "indigo__vector_stride_c";

        ROSE_ASSERT(isSgExpression(ref_node));
        SgExpression* expr = isSgExpression(ref_node);
        SgType* type = expr->get_type();
        SgSizeOfOp* size_of_op = new SgSizeOfOp(fileInfo, NULL, type, type);

        ROSE_ASSERT(size_of_op);

        std::vector<SgExpression*> params;
        params.push_back(param_line_number);
        params.push_back(param_idx);
        params.push_back(param_addr);
        params.push_back(size_of_op);

        statement_info_t statement_info;
        statement_info.statement = ir_methods::prepare_call_statement(
                containingBB, function_name, params, containingStmt);
        statement_info.reference_statement = containingStmt;
        statement_info.before = true;
        statement_list.push_back(statement_info);
    }
}

void vector_strides_t::process_loop(SgScopeStatement* outer_scope_stmt,
        loop_info_t& loop_info, expr_map_t& loop_map,
        name_list_t& stream_list) {
    int line_number = loop_info.loop_stmt->get_file_info()->get_raw_line();

    loop_map[loop_info.idxv_expr] = &loop_info;
    if (contains_non_linear_reference(loop_info.reference_list)) {
        std::cerr << mprefix << "Found non-linear reference(s) in loop." <<
            std::endl;
        return;
    }

    // Instrument this loop only if
    // the loop header components have been identified.
    // Allow empty init expressions (which is always the case with while and
    // do-while loops).
    if (loop_info.idxv_expr && loop_info.test_expr && loop_info.test_expr) {
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*)
                     outer_scope_stmt)->get_file_info()->get_filenameString());

        instrument_vector_strides(fileInfo, loop_info.loop_stmt);
    } else {
        // Instrument inner loops only if
        // the outer loop could not be instrumented.
        for(std::vector<loop_info_list_t>::iterator it =
                loop_info.child_loop_info.begin();
                it != loop_info.child_loop_info.end(); it++) {
            loop_info_list_t& loop_info_list = *it;
            for(loop_info_list_t::iterator it2 = loop_info_list.begin();
                    it2 != loop_info_list.end(); it2++) {
                loop_info_t& loop_info = *it2;
                process_loop(outer_scope_stmt, loop_info, loop_map, stream_list);
            }
        }
    }
}

void vector_strides_t::process_node(SgNode* node) {
    // Since this is not really a traversal, manually invoke init function.
    atTraversalStart();

    loop_traversal_t loop_traversal(var_renaming);
    loop_traversal.traverse(node, attrib());
    loop_info_list_t& loop_info_list = loop_traversal.get_loop_info_list();

    expr_map_t loop_map;
    name_list_t stream_list;

    for(loop_info_list_t::iterator it = loop_info_list.begin();
            it != loop_info_list.end(); it++) {
        loop_info_t& loop_info = *it;
        process_loop(loop_info.loop_stmt, loop_info, loop_map, stream_list);
    }

    // Since this is not really a traversal, manually invoke atTraversalEnd();
    atTraversalEnd();
}

const analysis_profile_t& vector_strides_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator vector_strides_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator vector_strides_t::stmt_end() {
    return statement_list.end();
}
