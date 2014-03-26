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

#include "tripcount.h"
#include "analysis_profile.h"
#include "generic_vars.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"

using namespace SageBuilder;
using namespace SageInterface;

tripcount_t::tripcount_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
    loop_traversal = new loop_traversal_t(var_renaming);
}

tripcount_t::~tripcount_t() {
    delete loop_traversal;
    loop_traversal = NULL;
}

void tripcount_t::atTraversalStart() {
    statement_list.clear();
}

void tripcount_t::atTraversalEnd() {
}

void tripcount_t::instrument_loop_trip_count(Sg_File_Info* fileInfo,
        loop_info_t& loop_info) {
    SgScopeStatement* scope_stmt = loop_info.loop_stmt;
    SgExpression* idxv = loop_info.idxv_expr;
    SgExpression* init = loop_info.init_expr;
    SgExpression* test = loop_info.test_expr;

    if (SgBasicBlock* bb = getEnclosingNode<SgBasicBlock>(scope_stmt)) {
        int line_number = scope_stmt->get_file_info()->get_raw_line();

        // Create new integer variable calledi
        // "indigo__trip_count_<line_number>". Funky, eh?
        char var_name[32];
        snprintf (var_name, 32, "indigo__trip_count_%d", line_number);
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
                params, scope_stmt);

        SgStatement* reference_stmt = NULL;
        reference_stmt = getEnclosingNode<SgScopeStatement>(scope_stmt);

        SgOmpBodyStatement* omp_body_stmt = NULL;
        omp_body_stmt = getEnclosingNode<SgOmpBodyStatement>(scope_stmt);
        if (omp_body_stmt && ir_methods::is_ancestor((SgNode*) reference_stmt,
                    (SgNode*) omp_body_stmt) == false) {
            reference_stmt = omp_body_stmt;
        }

        if (reference_stmt == NULL ||
                (ir_methods::is_loop(reference_stmt) == false &&
                isSgOmpBodyStatement(reference_stmt) == false)) {
            reference_stmt = scope_stmt;
        }

        statement_info_t tripcount_decl;
        tripcount_decl.statement = trip_count;
        tripcount_decl.reference_statement = reference_stmt;
        tripcount_decl.before = true;
        statement_list.push_back(tripcount_decl);

        statement_info_t tripcount_call;
        tripcount_call.statement = expr_statement;
        tripcount_call.reference_statement = reference_stmt;
        tripcount_call.before = false;
        statement_list.push_back(tripcount_call);

        statement_info_t tripcount_incr;
        tripcount_incr.statement = incr_statement;
        tripcount_incr.reference_statement = getFirstStatement(scope_stmt);
        tripcount_incr.before = true;
        statement_list.push_back(tripcount_incr);
    }
}

void tripcount_t::process_loop(SgScopeStatement* outer_scope_stmt,
        loop_info_t& loop_info, expr_map_t& loop_map,
        name_list_t& stream_list) {
    int line_number = loop_info.loop_stmt->get_file_info()->get_raw_line();
    loop_map[loop_info.idxv_expr] = &loop_info;

    // Instrument this loop only if
    // the loop header components have been identified.
    // Allow empty init expressions (which is always the case with while and
    // do-while loops).
    if (loop_info.idxv_expr && loop_info.test_expr && loop_info.test_expr /* &&
            !contains_non_linear_reference(loop_info.reference_list) */) {
        loop_info.processed = true;

        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*)
                     outer_scope_stmt)->get_file_info()->get_filenameString());

        instrument_loop_trip_count(fileInfo, loop_info);
    }

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

void tripcount_t::process_node(SgNode* node) {
    // Begin processing.
    analysis_profile.start_timer();

    // Since this is not really a traversal, manually invoke init function.
    atTraversalStart();

    loop_traversal->traverse(node, attrib());
    loop_info_list_t& loop_info_list = loop_traversal->get_loop_info_list();

    expr_map_t loop_map;
    name_list_t stream_list;

    for(loop_info_list_t::iterator it = loop_info_list.begin();
            it != loop_info_list.end(); it++) {
        loop_info_t& loop_info = *it;
        process_loop(loop_info.loop_stmt, loop_info, loop_map, stream_list);
    }

    // Since this is not really a traversal, manually invoke atTraversalEnd();
    atTraversalEnd();

    analysis_profile.end_timer();
    analysis_profile.set_loop_info_list(loop_info_list);
}

const analysis_profile_t& tripcount_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator tripcount_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator tripcount_t::stmt_end() {
    return statement_list.end();
}
