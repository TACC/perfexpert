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

#include "rose.h"
#include "ir_methods.h"
#include "aligncheck.h"

using namespace SageBuilder;
using namespace SageInterface;

aligncheck_t::aligncheck_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
}

void aligncheck_t::atTraversalStart() {
    inst_info_list.clear();
}

void aligncheck_t::atTraversalEnd() {
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

        if (init_expr)
            instrument_loop_header_components(node, def_map, init_expr,
                    LOOP_INIT);

        if (test_expr)
            instrument_loop_header_components(node, def_map, test_expr,
                    LOOP_TEST);

        if (incr_expr)
            instrument_loop_header_components(node, def_map, incr_expr,
                    LOOP_INCR);
    }
}

void aligncheck_t::instrument_loop_header_components(SgNode*& loop_node,
        def_map_t& def_map, SgExpression*& instrument_expr,
        short loop_inst_type) {

    std::string expr_str = instrument_expr->unparseToString();
    if (isSgVarRefExp(instrument_expr) && def_map.find(expr_str) !=
            def_map.end()) {
        VariableRenaming::NumNodeRenameEntry entry_list = def_map[expr_str];

        if (entry_list.size() == 1) {
            // Only one definition of this variable reaches here.
            // If it's value is a constant, we can directly use
            // the constant in place of the variable!
            SgNode* def_node = entry_list.begin()->second;
            if (ir_methods::get_expr_value(def_node, expr_str) == NULL)
                 insert_instrumentation(def_node, instrument_expr,
                        loop_inst_type, false);
        } else {
            // Instrument all definitions of this variable.
            for (entry_iterator entry_it = entry_list.begin();
                    entry_it != entry_list.end(); entry_it++) {
                SgNode* def_node = (*entry_it).second;
                insert_instrumentation(def_node, instrument_expr,
                        loop_inst_type, false);
            }
        }
    } else if (!isSgValueExp(instrument_expr) &&
            !isConstType(instrument_expr->get_type())) {
        // Instrument without using the reaching definition.
        insert_instrumentation(loop_node, instrument_expr, loop_inst_type,
                true);
    }
}

void aligncheck_t::insert_instrumentation(SgNode*& node, SgExpression*& expr,
        short type, bool before) {
    assert(isSgLocatedNode(node) &&
            "Cannot obtain file information for node to be instrumented.");

    Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
            ((SgLocatedNode*) node)->get_file_info()->get_filenameString());

    SgIntVal* param_type = new SgIntVal(fileInfo, type);

    SgBasicBlock* outer_basic_block = getEnclosingNode<SgBasicBlock>(node);
    SgStatement* outer_statement = isSgStatement(node) ? :
            getEnclosingNode<SgStatement>(node);

    assert(outer_basic_block);
    assert(outer_statement);

    inst_info_t inst_info;
    inst_info.bb = outer_basic_block;
    inst_info.stmt = outer_statement;
    inst_info.params.push_back(expr);
    inst_info.params.push_back(param_type);

    inst_info.function_name = SageInterface::is_Fortran_language() ? "indigo__aligncheck_f" : "indigo__aligncheck_c";
    inst_info.before = before;

    inst_info_list.push_back(inst_info);
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_begin() {
    return inst_info_list.begin();
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_end() {
    return inst_info_list.end();
}
