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
#include <string>
#include <vector>

#include "pntr_overlap.h"
#include "analysis_profile.h"
#include "generic_vars.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

pntr_overlap_t::pntr_overlap_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
    loop_traversal = new loop_traversal_t(var_renaming);
}

pntr_overlap_t::~pntr_overlap_t() {
    delete loop_traversal;
    loop_traversal = NULL;
}

void pntr_overlap_t::atTraversalStart() {
    statement_list.clear();
}

void pntr_overlap_t::atTraversalEnd() {
}

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

void pntr_overlap_t::instrument_overlap_checks(Sg_File_Info* fileInfo,
        SgScopeStatement* outer_scope_stmt, loop_info_t& loop_info,
        name_list_t& stream_list, expr_map_t& loop_map) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    reference_list_t& reference_list = loop_info.reference_list;
    int line_number = loop_stmt->get_file_info()->get_raw_line();

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
    expr_list.push_back(new SgIntVal(fileInfo, line_number));
    expr_list.push_back(new SgIntVal(fileInfo, params.size() / 2));
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
    statement_list.push_back(overlap_check_call);
}

void pntr_overlap_t::process_loop(SgScopeStatement* outer_scope_stmt,
        loop_info_t& loop_info, expr_map_t& loop_map,
        name_list_t& stream_list) {
    int line_number = loop_info.loop_stmt->get_file_info()->get_raw_line();
    loop_map[loop_info.idxv_expr] = &loop_info;

    // Instrument this loop only if
    // the loop header components have been identified.
    // Allow empty init expressions (which is always the case with while and
    // do-while loops).
    if (loop_info.idxv_expr && loop_info.test_expr && loop_info.incr_expr
            /* && !contains_non_linear_reference(loop_info.reference_list) */) {
        loop_info.processed = true;

        SgLocatedNode* located_outer_scope =
            reinterpret_cast<SgLocatedNode*>(outer_scope_stmt);
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    located_outer_scope->get_file_info()->get_filenameString());

        instrument_overlap_checks(fileInfo, outer_scope_stmt, loop_info,
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

void pntr_overlap_t::process_node(SgNode* node) {
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

const analysis_profile_t& pntr_overlap_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator pntr_overlap_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator pntr_overlap_t::stmt_end() {
    return statement_list.end();
}
