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

#include "inst_defs.h"
#include "ir_methods.h"
#include "loop_traversal.h"
#include "streams.h"

using namespace SageBuilder;
using namespace SageInterface;

loop_traversal_t::loop_traversal_t(VariableRenaming*& _var_renaming) {
    for_stmt = NULL;
    var_renaming = _var_renaming;
}

loop_info_list_t& loop_traversal_t::get_loop_info_list() {
    return loop_info_list;
}

attrib loop_traversal_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    if (attr.skip)
        return attr;

    if (SgForStatement* for_stmt = isSgForStatement(node)) {
        ir_methods::def_map_t def_map;
        SgExpression* idxv = NULL;
        SgExpression* init = NULL;
        SgExpression* test = NULL;
        SgExpression* incr = NULL;

        VariableRenaming::NumNodeRenameTable rename_table =
            var_renaming->getReachingDefsAtNode(node);

        // Expand the iterator list into a map for easier lookup.
        ir_methods::construct_def_map(rename_table, def_map);

        ir_methods::get_loop_header_components(var_renaming, for_stmt, def_map,
                idxv, init, test, incr);

        streams_t streams(false);
        streams.traverse(for_stmt, attrib());
        reference_list_t& _ref_list = streams.get_reference_list();

        loop_traversal_t inner_traversal(var_renaming);
        inner_traversal.traverse(for_stmt->get_loop_body(), attrib());
        loop_info_list_t& child_list = inner_traversal.get_loop_info_list();

        loop_info_t loop_info;
        loop_info.for_stmt = for_stmt;

        loop_info.idxv_expr = idxv;
        loop_info.init_expr = init;
        loop_info.test_expr = test;
        loop_info.incr_expr = incr;

        loop_info.reference_list = _ref_list;
        loop_info.child_loop_info.push_back(child_list);
        loop_info_list.push_back(loop_info);

        attr.skip = true;
        return attr;
    }

    return attr;
}
