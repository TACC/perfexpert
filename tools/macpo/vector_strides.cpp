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
#include <vector>

#include "generic_vars.h"
#include "ir_methods.h"
#include "vector_strides.h"

using namespace SageBuilder;
using namespace SageInterface;

bool vector_strides_t::contains_non_linear_reference(const reference_list_t&
        reference_list) {
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

bool vector_strides_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();

    if (contains_non_linear_reference(loop_info.reference_list)) {
        std::cerr << mprefix << "Found non-linear reference(s) in loop." <<
            std::endl;
        return false;
    }

    generic_vars_t generic_vars(true);
    generic_vars.traverse(loop_stmt, attrib());
    reference_list_t& var_list = generic_vars.get_reference_list();

    size_t count = 0;
    for (reference_list_t::iterator it = var_list.begin(); it != var_list.end();
            it++) {
        reference_info_t& reference_info = *it;
        std::string stream = reference_info.name;
        SgNode* ref_node = reference_info.node;

        if (count == reference_info.idx) {
            add_stream(stream);
            count += 1;
        }

        SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(ref_node);
        SgStatement* containingStmt = getEnclosingNode<SgStatement>(ref_node);

        int line_number = 0;
        SgStatement *stmt = getEnclosingNode<SgStatement>(ref_node);
        if (stmt) {
            line_number = stmt->get_file_info()->get_raw_line();
        }

        if (line_number == 0)
            continue;

        line_number = loop_stmt->get_file_info()->get_raw_line();

        SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
        SgIntVal* param_idx = new SgIntVal(fileInfo, reference_info.idx);
        param_line_number->set_endOfConstruct(fileInfo);
        param_idx->set_endOfConstruct(fileInfo);

        SgExpression* expr = isSgExpression(ref_node);
        ROSE_ASSERT(expr);

        // Strip unary operators like ++ or -- from the expression.
        SgExpression* stripped_expr = NULL;
        stripped_expr = ir_methods::strip_unary_operators(expr);
        ROSE_ASSERT(stripped_expr && "Bug in stripping unary operators "
                "from given expression!");

        // If not Fortran, cast the address to a void pointer
        SgExpression *param_addr = SageInterface::is_Fortran_language() ?
            stripped_expr : buildCastExp(
                    buildAddressOfOp(stripped_expr),
                    buildPointerType(buildVoidType()));

        std::string function_name = SageInterface::is_Fortran_language() ?
                "indigo__vector_stride_f" : "indigo__vector_stride_c";

        SgType* type = expr->get_type();
        SgSizeOfOp* size_of_op = new SgSizeOfOp(fileInfo, NULL, type, type);
        size_of_op->set_endOfConstruct(fileInfo);

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
        add_stmt(statement_info);
    }

    return true;
}
