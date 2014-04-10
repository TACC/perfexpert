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

#include "tripcount.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

void tripcount_t::instrument_loop(loop_info_t& loop_info) {
    SgScopeStatement* loop_stmt = loop_info.loop_stmt;
    Sg_File_Info* fileInfo = loop_stmt->get_file_info();

    SgExpression* idxv = loop_info.idxv_expr;
    SgExpression* init = loop_info.init_expr;
    SgExpression* test = loop_info.test_expr;

    if (SgBasicBlock* bb = getEnclosingNode<SgBasicBlock>(loop_stmt)) {
        int line_number = loop_stmt->get_file_info()->get_raw_line();

        // Create new integer variable calledi
        // "indigo__trip_count_<line_number>". Funky, eh?
        char var_name[32];
        snprintf(var_name, sizeof(var_name), "indigo__trip_count_%d",
                line_number);
        SgType* long_type = buildLongType();

        SgVariableDeclaration* trip_count = NULL;
        trip_count = ir_methods::create_long_variable(fileInfo, var_name, 0);
        trip_count->set_parent(bb);

        SgVarRefExp* trip_count_expr = buildVarRefExp(var_name);
        SgPlusPlusOp* incr_op = new SgPlusPlusOp(fileInfo,
                trip_count_expr, long_type);
        SgExprStatement* incr_statement = new SgExprStatement(fileInfo,
                incr_op);

        std::string function_name = SageInterface::is_Fortran_language()
            ? "indigo__tripcount_check_f" : "indigo__tripcount_check_c";

        std::vector<SgExpression*> params;
        params.push_back(buildIntVal(line_number));
        params.push_back(trip_count_expr);
        SgExprStatement* expr_statement = NULL;
        expr_statement = ir_methods::prepare_call_statement(bb, function_name,
                params, loop_stmt);

        SgStatement* reference_stmt = NULL;
        reference_stmt = getEnclosingNode<SgScopeStatement>(loop_stmt);

        SgOmpBodyStatement* omp_body_stmt = NULL;
        omp_body_stmt = getEnclosingNode<SgOmpBodyStatement>(loop_stmt);

        SgNode* ref_node = reinterpret_cast<SgNode*>(reference_stmt);
        SgNode* omp_node = reinterpret_cast<SgNode*>(omp_body_stmt);
        if (omp_body_stmt && ir_methods::is_ancestor(ref_node, omp_node)
                == false) {
            reference_stmt = omp_body_stmt;
        }

        if (reference_stmt == NULL ||
                (ir_methods::is_loop(reference_stmt) == false &&
                 isSgOmpBodyStatement(reference_stmt) == false)) {
            reference_stmt = loop_stmt;
        }

        statement_info_t tripcount_decl;
        tripcount_decl.statement = trip_count;
        tripcount_decl.reference_statement = reference_stmt;
        tripcount_decl.before = true;
        add_stmt(tripcount_decl);

        statement_info_t tripcount_call;
        tripcount_call.statement = expr_statement;
        tripcount_call.reference_statement = reference_stmt;
        tripcount_call.before = false;
        add_stmt(tripcount_call);

        statement_info_t tripcount_incr;
        tripcount_incr.statement = incr_statement;
        tripcount_incr.reference_statement = getFirstStatement(loop_stmt);
        tripcount_incr.before = true;
        add_stmt(tripcount_incr);
    }
}
