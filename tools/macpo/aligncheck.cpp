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

#include "aligncheck.h"
#include "ir_methods.h"
#include "streams.h"

using namespace SageBuilder;
using namespace SageInterface;

aligncheck_t::aligncheck_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
}

void aligncheck_t::atTraversalStart() {
    inst_info_list.clear();
}

void aligncheck_t::atTraversalEnd() {
    for (inst_list_t::iterator it = inst_info_list.begin(); it != inst_info_list.end(); it++) {
        ir_methods::insert_instrumentation_call(*it);
    }
}

void aligncheck_t::visit(SgNode* node) {
    SgExpression *idxv_expr, *init_expr, *test_expr, *incr_expr;

    SgForStatement* for_stmt = isSgForStatement(node);
    if (for_stmt) {
        def_map_t def_map;

        // If we need to instrument at least one scalar variable,
        // can this instrumentation be relocated to an earlier point
        // in the program that is outside all loops?
        VariableRenaming::NumNodeRenameTable rename_table =
            var_renaming->getReachingDefsAtNode(node);

        // Expand the iterator list into a map for easier lookup.
        ir_methods::construct_def_map(rename_table, def_map);

        // Extract components from the for-loop header.
        ir_methods::get_loop_header_components(var_renaming, for_stmt, def_map,
                idxv_expr, init_expr, test_expr, incr_expr);

        // Instrument the loop only if all headers are available.
        if ((init_expr || idxv_expr) && test_expr && incr_expr) {
            streams_t streams;
            streams.traverse(node, attrib());
            reference_list_t& reference_list = streams.get_reference_list();

            // Sanity check: Only linear array references.
            for (reference_list_t::iterator it = reference_list.begin();
                    it != reference_list.end(); it++) {
                reference_info_t& reference_info = *it;
                SgNode* ref_node = reference_info.node;

                // Check if ref_node is a linear array reference
                SgPntrArrRefExp* pntr = isSgPntrArrRefExp(ref_node);
                if (pntr && pntr->get_rhs_operand() &&
                        !isSgVarRefExp(pntr->get_rhs_operand())) {
                    // Non-linear array reference, cannot vectorize.
                    return;
                }
            }

            std::set<std::string> stream_set;

            typedef std::vector<SgPntrArrRefExp*> expr_list_t;
            expr_list_t expr_list;

            // Extract array references for alignment checking.
            for (reference_list_t::iterator it = reference_list.begin();
                    it != reference_list.end(); it++) {
                reference_info_t& reference_info = *it;
                SgNode* ref_node = reference_info.node;

                // Check if ref_node is a linear array reference
                SgPntrArrRefExp* pntr = isSgPntrArrRefExp(ref_node);
                if (pntr) {
                    std::string stream_name = pntr->unparseToString();
                    if (stream_set.find(stream_name) == stream_set.end()) {
                        stream_set.insert(stream_name);
                        SgPntrArrRefExp* copy_pntr = isSgPntrArrRefExp(
                                copyExpression(pntr));

                        if (copy_pntr)
                            expr_list.push_back(copy_pntr);
                    }
                }
            }

            std::string function_name = SageInterface::is_Fortran_language()
                ? "indigo__aligncheck_f" : "indigo__aligncheck_c";
            SgBasicBlock* outer_bb = getEnclosingNode<SgBasicBlock>(for_stmt);
            Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*) for_stmt)->get_file_info()->get_filenameString());
            int line_number = for_stmt->get_file_info()->get_raw_line();
            SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);

            for (expr_list_t::iterator it = expr_list.begin();
                    it != expr_list.end(); it++) {
                SgPntrArrRefExp*& expr = *it;

                // Replace idxv in expr with init value
                ir_methods::replace_expr(expr, idxv_expr, init_expr);

                inst_info_t inst_info;
                inst_info.bb = outer_bb;
                inst_info.stmt = for_stmt;
                SgExpression *param_addr = SageInterface::is_Fortran_language()
                        ? (SgExpression*) expr : buildCastExp (
                        buildAddressOfOp((SgExpression*) expr),
                        buildPointerType(buildVoidType()));

                inst_info.params.push_back(param_addr);
                inst_info.params.push_back(param_line_number);

                inst_info.function_name = function_name;

                // FIXME: Setting before to true causes AST to be repeatedly traversed.
                inst_info.before = false;
                inst_info_list.push_back(inst_info);
            }

            // Instrument the loop only if the compiler cannot vectorize it.
            if ((init_expr && !ir_methods::is_known(init_expr)) ||
                    !ir_methods::is_known(test_expr) ||
                    !ir_methods::is_known(test_expr)) {
            }

            if (init_expr) {
                instrument_loop_header_components(node, def_map, init_expr,
                        LOOP_INIT);
            } else {
                instrument_loop_header_components(node, def_map, idxv_expr,
                        LOOP_INIT);
            }

            instrument_loop_header_components(node, def_map, test_expr,
                    LOOP_TEST);

            instrument_loop_header_components(node, def_map, incr_expr,
                    LOOP_INCR);
        }
    }
}

void aligncheck_t::instrument_loop_header_components(SgNode*& loop_node,
        def_map_t& def_map, SgExpression*& instrument_expr,
        short loop_inst_type) {

    std::string expr_str = instrument_expr->unparseToString();
    if (loop_inst_type != LOOP_INIT && isSgVarRefExp(instrument_expr) &&
            def_map.find(expr_str) != def_map.end()) {
        VariableRenaming::NumNodeRenameEntry entry_list = def_map[expr_str];

        if (entry_list.size() == 1) {
            // Only one definition of this variable reaches here.
            // If it's value is a constant, we can directly use
            // the constant in place of the variable!
            SgNode* def_node = entry_list.begin()->second;
            insert_instrumentation(def_node, instrument_expr,
                    loop_inst_type, false, loop_node);
        } else {
            // FIXME: Use post-dominator tree, instrument only where required.
#if 0
            // Instrument all definitions of this variable.
            for (entry_iterator entry_it = entry_list.begin();
                    entry_it != entry_list.end(); entry_it++) {
                SgNode* def_node = (*entry_it).second;
                insert_instrumentation(def_node, instrument_expr,
                        loop_inst_type, false, loop_node);
            }
#else
            // Instrument just before the loop without reaching def analysis..
            if (!isSgValueExp(instrument_expr) &&
                    !isConstType(instrument_expr->get_type())) {
                insert_instrumentation(loop_node, instrument_expr,
                        loop_inst_type, true, loop_node);
            }
#endif
        }
    } else if (!isSgValueExp(instrument_expr) &&
            !isConstType(instrument_expr->get_type())) {
        // Instrument without using the reaching definition.
        insert_instrumentation(loop_node, instrument_expr, loop_inst_type,
                true, loop_node);
    }
}

void aligncheck_t::insert_instrumentation(SgNode*& node, SgExpression*& expr,
        short type, bool before, SgNode*& loop_node) {
    assert(isSgLocatedNode(node) &&
            "Cannot obtain file information for node to be instrumented.");

    std::string function_name;
    switch(type) {
        case LOOP_INIT:
            function_name = SageInterface::is_Fortran_language() ?
                    "indigo__initcheck_f" : "indigo__initcheck_c";
            break;

        case LOOP_TEST:
            function_name = SageInterface::is_Fortran_language() ?
                    "indigo__testcheck_f" : "indigo__testcheck_c";
            break;

        case LOOP_INCR:
            function_name = SageInterface::is_Fortran_language() ?
                    "indigo__incrcheck_f" : "indigo__incrcheck_c";
            break;

        default:
            std::cerr << "Invalid loop type value." << std::endl;
            return;
    }

    Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
            ((SgLocatedNode*) node)->get_file_info()->get_filenameString());

    SgBasicBlock* outer_basic_block = getEnclosingNode<SgBasicBlock>(node);
    SgStatement* outer_statement = isSgStatement(node) ? :
            getEnclosingNode<SgStatement>(node);

    SgStatement* loop_statement = isSgStatement(loop_node) ? :
            getEnclosingNode<SgStatement>(loop_node);

    assert(outer_basic_block);
    assert(outer_statement);

    int line_number = 0;

    // Statement that represents the loop.
    if (!loop_statement) {
        std::cerr << "Failed to find the statement corresponding to loop," <<
            " continuing..." << std::endl;
    }

    line_number = loop_statement->get_file_info()->get_raw_line();
    SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);

    inst_info_t inst_info;
    inst_info.bb = outer_basic_block;
    inst_info.stmt = outer_statement;
    inst_info.params.push_back(expr);
    inst_info.params.push_back(param_line_number);

    inst_info.function_name = function_name;
    inst_info.before = before;

    inst_info_list.push_back(inst_info);
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_begin() {
    return inst_info_list.begin();
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_end() {
    return inst_info_list.end();
}
