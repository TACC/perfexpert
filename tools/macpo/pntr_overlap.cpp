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

#include "pntr_overlap.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

void pntr_overlap_t::create_spans_for_child_loops(SgExpression* expr_init,
        SgExpression* expr_term, loop_info_t& loop_info) {
    ir_methods::create_spans(expr_init, expr_term, loop_info);
    for (std::vector<loop_info_list_t>::iterator it =
            loop_info.child_loop_info.begin();
            it != loop_info.child_loop_info.end(); it++) {
        loop_info_list_t& loop_info_list = *it;
        for (loop_info_list_t::iterator it = loop_info_list.begin();
                it != loop_info_list.end(); it++) {
            loop_info_t& loop_info = *it;
            create_spans_for_child_loops(expr_init, expr_term, loop_info);
        }
    }
}

void pntr_overlap_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();
    int line_number = fileInfo->get_raw_line();

    reference_list_t& reference_list = loop_info.reference_list;

    // Is there anything to instrument?
    if (reference_list.size() == 0)
        return;

    SgStatement* loop_body = loop_stmt->firstStatement();
    SgBasicBlock* loop_bb = getEnclosingNode<SgBasicBlock>(loop_body);
    SgBasicBlock* aligncheck_list = new SgBasicBlock(fileInfo);

    SgExpression* idxv = loop_info.idxv_expr;
    SgExpression* init = loop_info.init_expr;
    SgExpression* incr = loop_info.incr_expr;
    int incr_op = loop_info.incr_op;

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

    expr_list_t params;
    for (expr_list_t::iterator it = expr_list.begin(); it != expr_list.end();
            it++) {
        SgExpression* expr = *it;

        if (expr) {
            SgExpression* copy = copyExpression(expr);
            ROSE_ASSERT(copy);
            create_spans_for_child_loops(expr, copy, loop_info);

            if (is_Fortran_language()) {
                params.push_back(expr);
                params.push_back(copy);
            } else {
                SgCastExp* cast_expr_01 = NULL;
                cast_expr_01 = buildCastExp(buildAddressOfOp(expr),
                        buildPointerType(buildVoidType()));

                SgCastExp* cast_expr_02 = NULL;
                cast_expr_02 = buildCastExp(buildAddressOfOp(copy),
                        buildPointerType(buildVoidType()));

                params.push_back(cast_expr_01);
                params.push_back(cast_expr_02);
            }
        }
    }

    expr_list.clear();
    SgIntVal* val_line_number = new SgIntVal(fileInfo, line_number);
    SgIntVal* val_params = new SgIntVal(fileInfo, params.size() / 2);
    val_line_number->set_endOfConstruct(fileInfo);
    val_params->set_endOfConstruct(fileInfo);

    expr_list.push_back(val_line_number);
    expr_list.push_back(val_params);
    expr_list.insert(expr_list.end(), params.begin(), params.end());

    params.clear();

    // One last thing before adding the instrumentation call.
    // If there are any variable declarations in
    // the for-initialization, then hoist them outside the for loop.
    if (SgForStatement* for_stmt = isSgForStatement(loop_stmt)) {
        std::vector<SgVariableDeclaration*> decl_list =
            ir_methods::get_var_decls(for_stmt);

        for (std::vector<SgVariableDeclaration*>::iterator it =
                decl_list.begin(); it != decl_list.end(); it++) {
            SgVariableDeclaration* decl = *it;
            removeStatement(decl);
            insertStatementBefore(loop_stmt, decl);
            ir_methods::match_end_of_constructs(loop_stmt, decl);
        }
    }

    std::string function_name = SageInterface::is_Fortran_language()
        ? "indigo__overlap_check_f" : "indigo__overlap_check_c";
    SgStatement* call_stmt = ir_methods::prepare_call_statement(loop_bb,
            function_name, expr_list, loop_stmt);

    expr_list_t param_list;
    statement_info_t overlap_check_call;
    overlap_check_call.reference_statement = loop_stmt;
    overlap_check_call.statement = call_stmt;
    overlap_check_call.before = false;
    add_stmt(overlap_check_call);
}
