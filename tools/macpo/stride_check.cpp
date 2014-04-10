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

#include "ir_methods.h"
#include "stride_check.h"

using namespace SageBuilder;
using namespace SageInterface;

bool stride_check_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();
    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return true;

    SgStatement* loop_body = loop_stmt->firstStatement();
    SgExpression* idxv = loop_info.idxv_expr;
    SgExpression* init = loop_info.init_expr;
    SgExpression* incr = loop_info.incr_expr;
    int incr_op = loop_info.incr_op;

    if (idxv == NULL || incr == NULL || incr_op == ir_methods::INVALID_OP)
        return false;

    // Save expressions after stripping unary operands.
    expr_list_t expr_list;
    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        if (SgExpression* expr = isSgExpression(reference_info.node)) {
            SgExpression* copy = copyExpression(expr);
            SgExpression* stripped_expr = NULL;
            stripped_expr = ir_methods::strip_unary_operators(copy);
            expr_list.push_back(stripped_expr);
        }
    }

    ir_methods::remove_duplicate_expressions(expr_list);

    ir_methods::def_map_t def_map;

    // Check if this variable reference has just one definition.
    VariableRenaming::NumNodeRenameTable rename_table = get_defs_at_node(
            loop_stmt);

    // Expand the iterator list into a map for easier lookup.
    ir_methods::construct_def_map(rename_table, def_map);

    expr_list_t params;
    for (expr_list_t::iterator it = expr_list.begin(); it != expr_list.end();
            it++) {
        SgExpression* expr = *it;
        if (SgPntrArrRefExp* pntr = isSgPntrArrRefExp(expr)) {
            SgBinaryOp* bin_op = reinterpret_cast<SgBinaryOp*>(pntr);
            SgExpression* index_expr = bin_op->get_rhs_operand();

            // If non-linear reference, set stride to UNKNOWN.
            if (ir_methods::is_linear_reference(pntr, false) == false) {
                record_unknown_stride(loop_stmt, expr);
            } else if (isConstType(index_expr->get_type()) == true) {
                SgIntVal* val_zero = new SgIntVal(fileInfo, 0);
                val_zero->set_endOfConstruct(fileInfo);
                record_stride_value(loop_stmt, expr, val_zero);
            } else {
                SgExpression* copy_01 = copyExpression(index_expr);
                SgExpression* copy_02 = copyExpression(index_expr);

                ROSE_ASSERT(copy_01 && copy_02 && "Failed to copy expression!");

                if (init)
                    ir_methods::replace_expr(copy_01, idxv, init);

                ir_methods::replace_expr(copy_02, idxv, incr);

                SgSubtractOp* sub_op = new SgSubtractOp(fileInfo, copy_02,
                        copy_01, copy_01->get_type());
                sub_op->set_endOfConstruct(fileInfo);
                ir_methods::match_end_of_constructs(loop_stmt, sub_op);
                record_stride_value(loop_stmt, expr, sub_op);
            }
        }
    }

    return true;
}

void stride_check_t::record_unknown_stride(SgScopeStatement* loop_stmt,
        SgExpression* expr) {
    int line_number = loop_stmt->get_file_info()->get_raw_line();
    SgIntVal* line_number_val = new SgIntVal(loop_stmt->get_file_info(),
            line_number);
    line_number_val->set_endOfConstruct(loop_stmt->get_endOfConstruct());

    expr_list_t expr_list;
    expr_list.push_back(line_number_val);

    SgBasicBlock* loop_bb = getEnclosingNode<SgBasicBlock>(loop_stmt);
    std::string function_name = SageInterface::is_Fortran_language()
        ? "indigo__unknown_stride_check_f" : "indigo__unknown_stride_check_c";
    SgStatement* call_stmt = ir_methods::prepare_call_statement(loop_bb,
            function_name, expr_list, loop_stmt);

    statement_info_t stride_check_call;
    stride_check_call.reference_statement = loop_stmt;
    stride_check_call.statement = call_stmt;
    stride_check_call.before = false;
    add_stmt(stride_check_call);
}

void stride_check_t::record_stride_value(SgScopeStatement* loop_stmt,
        SgExpression* expr, SgExpression* stride) {
    int line_number = loop_stmt->get_file_info()->get_raw_line();
    SgIntVal* line_number_val = new SgIntVal(loop_stmt->get_file_info(),
            line_number);
    line_number_val->set_endOfConstruct(loop_stmt->get_endOfConstruct());

    expr_list_t expr_list;
    expr_list.push_back(line_number_val);
    expr_list.push_back(stride);

    SgBasicBlock* loop_bb = getEnclosingNode<SgBasicBlock>(loop_stmt);
    std::string function_name = SageInterface::is_Fortran_language()
        ? "indigo__stride_check_f" : "indigo__stride_check_c";
    SgStatement* call_stmt = ir_methods::prepare_call_statement(loop_bb,
            function_name, expr_list, loop_stmt);

    statement_info_t stride_check_call;
    stride_check_call.reference_statement = loop_stmt;
    stride_check_call.statement = call_stmt;
    stride_check_call.before = false;
    add_stmt(stride_check_call);
}
