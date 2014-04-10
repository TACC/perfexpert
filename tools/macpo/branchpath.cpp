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

bool branchpath_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();

    SgStatement* first_stmt = getFirstStatement(loop_stmt);
    SgStatement* stmt = first_stmt;

    SgStatement* reference_stmt = NULL;
    reference_stmt = getEnclosingNode<SgScopeStatement>(loop_stmt);

    SgOmpBodyStatement* omp_body_stmt = NULL;
    omp_body_stmt = getEnclosingNode<SgOmpBodyStatement>(loop_stmt);
    SgNode* ref_node = reinterpret_cast<SgNode*>(reference_stmt);
    SgNode* omp_node = reinterpret_cast<SgNode*>(omp_body_stmt);
    if (omp_body_stmt && ir_methods::is_ancestor(ref_node, omp_node) == false) {
        reference_stmt = omp_body_stmt;
    }

    if (reference_stmt == NULL ||
            (ir_methods::is_loop(reference_stmt) == false &&
            isSgOmpBodyStatement(reference_stmt) == false)) {
        reference_stmt = loop_stmt;
    }

    while (stmt) {
        if (SgIfStmt* if_stmt = isSgIfStmt(stmt)) {
            int line_number = if_stmt->get_file_info()->get_raw_line();
            int for_line_number = loop_stmt->get_file_info()->get_raw_line();
            SgStatement* true_body = if_stmt->get_true_body();
            SgStatement* false_body = if_stmt->get_false_body();

            // Create two variables for each branch.
            SgVariableDeclaration *var_true = NULL, *var_false = NULL;

            char var_true_name[32], var_false_name[32];
            snprintf(var_true_name, sizeof(var_true_name),
                    "indigo__branch_true_%d", line_number);
            snprintf (var_false_name, sizeof(var_false_name),
                    "indigo__branch_false_%d", line_number);

            SgBasicBlock* outer_bb = getEnclosingNode<SgBasicBlock>(loop_stmt);
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
            add_stmt(true_decl);

            statement_info_t false_decl;
            false_decl.before = true;
            false_decl.statement = var_false;
            false_decl.reference_statement = reference_stmt;
            add_stmt(false_decl);

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
                add_stmt(true_stmt);
            }

            if (false_body == NULL) {
                if_stmt->set_false_body(false_incr);
            } else {
                statement_info_t false_stmt;
                false_stmt.statement = false_incr;
                false_stmt.reference_statement = false_body;
                false_stmt.before = true;
                add_stmt(false_stmt);
            }

            std::string function_name = SageInterface::is_Fortran_language()
                ? "indigo__record_branch_f" : "indigo__record_branch_c";

            std::vector<SgExpression*> params;
            SgIntVal* val_line_number = new SgIntVal(fileInfo, line_number);
            SgIntVal* val_for_line_number = new SgIntVal(fileInfo,
                    for_line_number);

            val_line_number->set_endOfConstruct(fileInfo);
            val_for_line_number->set_endOfConstruct(fileInfo);

            params.push_back(val_line_number);
            params.push_back(val_for_line_number);
            params.push_back(buildVarRefExp(var_true_name));
            params.push_back(buildVarRefExp(var_false_name));

            statement_info_t branch_statement;
            branch_statement.statement = ir_methods::prepare_call_statement(
                    getEnclosingNode<SgBasicBlock>(loop_stmt), function_name,
                    params, loop_stmt);
            branch_statement.reference_statement = reference_stmt;
            branch_statement.before = false;
            add_stmt(branch_statement);
        }

        stmt = getNextStatement(stmt);
    }

    return true;
}
