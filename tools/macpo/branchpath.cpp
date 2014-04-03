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

#include "branchpath.h"
#include "analysis_profile.h"
#include "generic_vars.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

branchpath_t::branchpath_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
    loop_traversal = new loop_traversal_t(var_renaming);
}

branchpath_t::~branchpath_t() {
    delete loop_traversal;
    loop_traversal = NULL;
}

void branchpath_t::atTraversalStart() {
    statement_list.clear();
}

void branchpath_t::atTraversalEnd() {
}

void branchpath_t::instrument_branches(Sg_File_Info* fileInfo,
        SgScopeStatement* scope_stmt, SgExpression* idxv_expr) {
    SgStatement* first_stmt = getFirstStatement(scope_stmt);
    SgStatement* stmt = first_stmt;

    SgStatement* reference_stmt = NULL;
    reference_stmt = getEnclosingNode<SgScopeStatement>(scope_stmt);

    SgOmpBodyStatement* omp_body_stmt = NULL;
    omp_body_stmt = getEnclosingNode<SgOmpBodyStatement>(scope_stmt);
    SgNode* ref_node = reinterpret_cast<SgNode*>(reference_stmt);
    SgNode* omp_node = reinterpret_cast<SgNode*>(omp_body_stmt);
    if (omp_body_stmt && ir_methods::is_ancestor(ref_node, omp_node) == false) {
        reference_stmt = omp_body_stmt;
    }

    if (reference_stmt == NULL ||
            (ir_methods::is_loop(reference_stmt) == false &&
            isSgOmpBodyStatement(reference_stmt) == false)) {
        reference_stmt = scope_stmt;
    }

    while (stmt) {
        if (SgIfStmt* if_stmt = isSgIfStmt(stmt)) {
            int line_number = if_stmt->get_file_info()->get_raw_line();
            int for_line_number = scope_stmt->get_file_info()->get_raw_line();
            SgStatement* true_body = if_stmt->get_true_body();
            SgStatement* false_body = if_stmt->get_false_body();

            // Create two variables for each branch.
            SgVariableDeclaration *var_true = NULL, *var_false = NULL;

            char var_true_name[32], var_false_name[32];
            snprintf(var_true_name, sizeof(var_true_name),
                    "indigo__branch_true_%d", line_number);
            snprintf (var_false_name, sizeof(var_false_name),
                    "indigo__branch_false_%d", line_number);

            SgBasicBlock* outer_bb = getEnclosingNode<SgBasicBlock>(
                    scope_stmt);
            var_true = ir_methods::create_long_variable(fileInfo, var_true_name,
                    0);
            var_true->set_parent(outer_bb);
            var_false = ir_methods::create_long_variable(fileInfo,
                    var_false_name, 0);
            var_false->set_parent(outer_bb);

            statement_info_t true_decl;
            true_decl.before = true;
            true_decl.statement = var_true;
            true_decl.reference_statement = reference_stmt;
            statement_list.push_back(true_decl);

            statement_info_t false_decl;
            false_decl.before = true;
            false_decl.statement = var_false;
            false_decl.reference_statement = reference_stmt;
            statement_list.push_back(false_decl);

            SgExprStatement *true_incr = NULL, *false_incr = NULL;
            true_incr = ir_methods::create_long_incr_statement(fileInfo,
                    var_true_name);
            false_incr = ir_methods::create_long_incr_statement(fileInfo,
                    var_false_name);
            true_incr->set_parent(if_stmt);
            false_incr->set_parent(if_stmt);

            if (true_body == NULL) {
                if_stmt->set_true_body(true_incr);
            } else {
                statement_info_t true_stmt;
                true_stmt.statement = true_incr;
                true_stmt.reference_statement = true_body;
                true_stmt.before = true;
                statement_list.push_back(true_stmt);
            }

            if (false_body == NULL) {
                if_stmt->set_false_body(false_incr);
            } else {
                statement_info_t false_stmt;
                false_stmt.statement = false_incr;
                false_stmt.reference_statement = false_body;
                false_stmt.before = true;
                statement_list.push_back(false_stmt);
            }

            std::string function_name = SageInterface::is_Fortran_language()
                ? "indigo__record_branch_f" : "indigo__record_branch_c";

            std::vector<SgExpression*> params;
            params.push_back(new SgIntVal(fileInfo, line_number));
            params.push_back(new SgIntVal(fileInfo, for_line_number));
            params.push_back(buildVarRefExp(var_true_name));
            params.push_back(buildVarRefExp(var_false_name));

            statement_info_t branch_statement;
            branch_statement.statement = ir_methods::prepare_call_statement(
                    getEnclosingNode<SgBasicBlock>(scope_stmt),
                    function_name, params, scope_stmt);
            branch_statement.reference_statement = reference_stmt;
            branch_statement.before = false;
            statement_list.push_back(branch_statement);
        }

        stmt = getNextStatement(stmt);
    }
}

void branchpath_t::process_loop(SgScopeStatement* outer_scope_stmt,
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

        instrument_branches(fileInfo, loop_info.loop_stmt, loop_info.idxv_expr);
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

void branchpath_t::process_node(SgNode* node) {
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

const analysis_profile_t& branchpath_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator branchpath_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator branchpath_t::stmt_end() {
    return statement_list.end();
}
