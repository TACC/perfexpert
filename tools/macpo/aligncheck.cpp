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

#include "aligncheck.h"
#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"
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
    for (inst_list_t::iterator it = inst_info_list.begin();
            it != inst_info_list.end(); it++) {
        ir_methods::insert_instrumentation_call(*it);
    }
}

bool aligncheck_t::contains_non_linear_reference(
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

void aligncheck_t::process_loop(SgForStatement* outer_for_stmt,
        loop_info_t& loop_info, expr_map_t& loop_map,
        name_list_t& stream_list) {
    pntr_list_t pntr_list;
    std::set<std::string> stream_set;

    SgForStatement* for_stmt = loop_info.for_stmt;

    SgExpression* idxv = loop_info.idxv_expr;
    SgExpression* init = loop_info.init_expr;
    SgExpression* test = loop_info.test_expr;

    loop_map[idxv] = &loop_info;

    reference_list_t& reference_list = loop_info.reference_list;
    if (contains_non_linear_reference(reference_list)) {
        std::cout << "Found non-linear reference(s) in loop.\n";
        return;
    }

    Sg_File_Info *fileInfo =
        Sg_File_Info::generateFileInfoForTransformationNode(
                ((SgLocatedNode*)
                 outer_for_stmt)->get_file_info()->get_filenameString());

    // Extract streaming stores from array references in the loop.
    sstore_map_t sstore_map;
    for (reference_list_t::iterator it2 = reference_list.begin();
            it2 != reference_list.end(); it2++) {
        reference_info_t& reference_info = *it2;
        std::string& ref_stream = reference_info.name;

        switch (reference_info.access_type) {
            case TYPE_WRITE:
                sstore_map[ref_stream].push_back(reference_info.node);
                break;

            case TYPE_READ:
            case TYPE_READ_AND_WRITE:
            case TYPE_UNKNOWN:  // Handle unknown type the same way as a read.
                sstore_map_t::iterator it = sstore_map.find(ref_stream);
                if (it != sstore_map.end()) {
                    sstore_map.erase(it);
                }

                break;
        }
    }

    for (sstore_map_t::iterator it = sstore_map.begin(); it != sstore_map.end();
            it++) {
        const std::string& key = it->first;
        node_list_t& value = it->second;

        // Replace index variable with init expression.
        expr_list_t expr_list;
        for (node_list_t::iterator it2 = value.begin(); it2 != value.end();
                it2++) {
            SgExpression* node = isSgExpression(*it2);
            if (node) {
                SgBinaryOp* copy_node = isSgBinaryOp(copyExpression(node));
                for (expr_map_t::iterator it2 = loop_map.begin();
                        it2 != loop_map.end(); it2++) {
                    SgExpression* idxv = it2->first;
                    loop_info_t* loop_info = it2->second;

                    SgExpression* init = loop_info->init_expr;

                    if (ir_methods::contains_expr(copy_node, idxv)) {
                        ir_methods::replace_expr(copy_node, idxv, init);
                    }
                }

                expr_list.push_back(copy_node);

            }
        }

        // Remove duplicates.
        std::map<std::string, SgExpression*> string_expr_map;
        for (expr_list_t::iterator it2 = expr_list.begin();
                it2 != expr_list.end(); it2++) {
            SgExpression* expr = *it2;
            std::string expr_string = expr->unparseToString();
            if (string_expr_map.find(expr_string) == string_expr_map.end()) {
                string_expr_map[expr_string] = expr;
            } else {
                // An expression with the same string representation has
                // already been seen, so we skip this expression.
            }
        }

        // Reuse the same expression list from earlier.
        expr_list.clear();

        // Finally, loop over the map to get the unique expressions.
        for (std::map<std::string, SgExpression*>::iterator it2 =
                string_expr_map.begin(); it2 != string_expr_map.end(); it2++) {
            SgExpression* expr = it2->second;

            SgExpression *addr_expr =
                SageInterface::is_Fortran_language() ?
                (SgExpression*) expr : buildCastExp (
                        buildAddressOfOp((SgExpression*) expr),
                        buildPointerType(buildVoidType()));

            expr_list.push_back(addr_expr);
        }

        // If we have any expressions, add the instrumentation call.
        if (expr_list.size()) {
            int line_number = for_stmt->get_file_info()->get_raw_line();
            SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
            SgIntVal* param_count = new SgIntVal(fileInfo, expr_list.size());

            std::string function_name = SageInterface::is_Fortran_language()
                ? "indigo__sstore_aligncheck_f" : "indigo__sstore_aligncheck_c";

            inst_info_t inst_info;
            inst_info.stmt = for_stmt;
            inst_info.bb = getEnclosingNode<SgBasicBlock>(for_stmt);
            inst_info.params.push_back(param_line_number);
            inst_info.params.push_back(param_count);
            inst_info.params.insert(inst_info.params.end(), expr_list.begin(),
                    expr_list.end());
            inst_info.function_name = function_name;
            inst_info.before = true;

            inst_info_list.push_back(inst_info);
        }
    }

    for (reference_list_t::iterator it2 = reference_list.begin();
            it2 != reference_list.end(); it2++) {
        reference_info_t& reference_info = *it2;
        SgNode* ref_node = reference_info.node;

        SgPntrArrRefExp* pntr = isSgPntrArrRefExp(ref_node);

        if (pntr) {
            std::string stream_name = pntr->unparseToString();
            // FIXME: Rather rudimentary check, using just the stream name
            // instead of using the index expression as well.
            if (std::find(stream_list.begin(), stream_list.end(), stream_name)
                    == stream_list.end()) {
                stream_list.push_back(stream_name);

                if (stream_set.find(stream_name) == stream_set.end()) {
                    stream_set.insert(stream_name);
                    SgPntrArrRefExp* copy_pntr = isSgPntrArrRefExp(
                            copyExpression(pntr));

                    if (copy_pntr)
                        pntr_list.push_back(copy_pntr);
                }
            }
        }
    }

    std::string function_name = SageInterface::is_Fortran_language()
        ? "indigo__aligncheck_f" : "indigo__aligncheck_c";
    SgBasicBlock* outer_bb = getEnclosingNode<SgBasicBlock>(for_stmt);

    std::set<std::string> expr_set;
    expr_list_t param_list;
    for (pntr_list_t::iterator it = pntr_list.begin(); it != pntr_list.end();
            it++) {
        SgPntrArrRefExp*& init_ref = *it;

        if (init_ref) {
            SgBinaryOp* bin_op = isSgBinaryOp(init_ref);
            SgBinaryOp* bin_copy = isSgBinaryOp(copyExpression(bin_op));

            if (bin_op && bin_copy) {
                // Replace all known index variables
                // with initial and final values.
                for (expr_map_t::iterator it2 = loop_map.begin();
                        it2 != loop_map.end(); it2++) {
                    SgExpression* idxv = it2->first;
                    loop_info_t* loop_info = it2->second;

                    SgExpression* init = loop_info->init_expr;
                    SgExpression* test = loop_info->test_expr;
                    SgExpression* incr = loop_info->incr_expr;
                    int incr_op = loop_info->incr_op;

                    if (ir_methods::contains_expr(bin_op, idxv)) {
                        if (!isSgValueExp(init))
                            expr_set.insert(init->unparseToString());

                        ir_methods::replace_expr(bin_op, idxv, init);
                    }

                    if (ir_methods::contains_expr(bin_copy, idxv)) {
                        if (!isSgValueExp(test))
                            expr_set.insert(test->unparseToString());

                        // Construct final value based on
                        // the expressions: test, incr_op and incr_expr.
                        SgExpression* final_value = NULL;
                        final_value = ir_methods::get_final_value(fileInfo,
                                test, incr, incr_op);
                        assert(final_value);

                        ir_methods::replace_expr(bin_copy, idxv, final_value);
                    }
                }

                SgExpression *param_addr_initial =
                    SageInterface::is_Fortran_language() ?
                    (SgExpression*) bin_op : buildCastExp (
                            buildAddressOfOp((SgExpression*) bin_op),
                            buildPointerType(buildVoidType()));

                SgExpression *param_addr_final =
                    SageInterface::is_Fortran_language() ?
                    (SgExpression*) bin_copy : buildCastExp (
                            buildAddressOfOp((SgExpression*) bin_copy),
                            buildPointerType(buildVoidType()));

                assert(param_addr_initial);
                assert(param_addr_final);

                param_list.push_back(param_addr_initial);
                param_list.push_back(param_addr_final);
            } else {
                std::cout << "Failed to create copy.\n";
            }
        } else {
            std::cout << "NULL init_ref\n";
        }
    }

    if (pntr_list.size()) {
        int line_number = outer_for_stmt->get_file_info()->get_raw_line();

        SgIntVal* param_count = new SgIntVal(fileInfo, param_list.size() / 2);
        SgIntVal* param_line_no = new SgIntVal(fileInfo, line_number);

        // FIXME: Handle more than one expressions.
        if (expr_set.size() == 1) {
            def_map_t def_map;
            std::string expr = *(expr_set.begin());

            VariableRenaming::NumNodeRenameTable rename_table =
                var_renaming->getReachingDefsAtNode(for_stmt);

            // Expand the iterator list into a map for easier lookup.
            ir_methods::construct_def_map(rename_table, def_map);

            if (def_map.find(expr) != def_map.end()) {
                VariableRenaming::NumNodeRenameEntry entry_list = def_map[expr];
                for (entry_iterator entry_it = entry_list.begin();
                        entry_it != entry_list.end(); entry_it++) {
                    SgNode* def_node = (*entry_it).second;
                    if (isSgInitializedName(def_node)) {
                        // Instrument at start of loop.
                        inst_info_t inst_info;
                        inst_info.stmt = outer_for_stmt;
                        inst_info.bb =
                            getEnclosingNode<SgBasicBlock>(outer_for_stmt);
                        inst_info.params.push_back(param_line_no);
                        inst_info.params.push_back(param_count);
                        inst_info.params.insert(inst_info.params.end(),
                                param_list.begin(), param_list.end());
                        inst_info.function_name = function_name;
                        inst_info.before = true;

                        inst_info_list.push_back(inst_info);
                    } else {

                        inst_info_t inst_info;
                        inst_info.stmt = isSgStatement(def_node) ?
                            isSgStatement(def_node) :
                            getEnclosingNode<SgStatement>(def_node);
                        inst_info.bb = getEnclosingNode<SgBasicBlock>(def_node);
                        inst_info.params.push_back(param_line_no);
                        inst_info.params.push_back(param_count);
                        inst_info.params.insert(inst_info.params.end(),
                                param_list.begin(), param_list.end());
                        inst_info.function_name = function_name;
                        inst_info.before = false;

                        inst_info_list.push_back(inst_info);
                    }
                }
            } else {
                std::cout << "no definitions, placing before outermost for loop.\n";
                // TODO: Place function call before the start of the outermost loop.
                inst_info_t inst_info;
                inst_info.stmt = outer_for_stmt;
                inst_info.bb = getEnclosingNode<SgBasicBlock>(outer_for_stmt);
                inst_info.params.push_back(param_line_no);
                inst_info.params.push_back(param_count);
                inst_info.params.insert(inst_info.params.end(),
                        param_list.begin(), param_list.end());
                inst_info.function_name = function_name;
                inst_info.before = true;

                inst_info_list.push_back(inst_info);
            }
        }
    }

    for(std::vector<loop_info_list_t>::iterator it =
                loop_info.child_loop_info.begin();
                it != loop_info.child_loop_info.end(); it++) {
        loop_info_list_t& loop_info_list = *it;
        for(loop_info_list_t::iterator it2 = loop_info_list.begin();
                it2 != loop_info_list.end(); it2++) {
            loop_info_t& loop_info = *it2;
            process_loop(outer_for_stmt, loop_info, loop_map, stream_list);
        }
    }
}

void aligncheck_t::process_node(SgNode* node) {
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
        process_loop(loop_info.for_stmt, loop_info, loop_map, stream_list);
    }

    // Since this is not really a traversal, manually invoke atTraversalEnd();
    atTraversalEnd();
}
