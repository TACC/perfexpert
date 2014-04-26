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
#include <map>
#include <string>

#include "inst_defs.h"
#include "ir_methods.h"
#include "macpo_record.h"
#include "streams.h"

using namespace SageBuilder;
using namespace SageInterface;

streams_t::streams_t(bool _deep_search) {
    init_scope_stmt = NULL;
    deep_search = _deep_search;
}

void streams_t::atTraversalStart() {
    reference_list.clear();
}

void streams_t::atTraversalEnd() {
    size_t count = 0;
    std::map<std::string, size_t> stream_idx_map;

    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        const std::string& stream = reference_info.name;

        if (stream_idx_map.find(stream) == stream_idx_map.end()) {
            stream_idx_map[stream] = count;
            reference_info.idx = count;
            count += 1;
        } else {
            reference_info.idx = stream_idx_map[stream];
        }
    }
}

reference_list_t& streams_t::get_reference_list() {
    return reference_list;
}

attrib streams_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    // If explicit instructions to skip this node, then just return
    if (attr.skip)
        return attr;

    attr.access_type = TYPE_UNKNOWN;
    SgNode* parent = node->get_parent();

    if (deep_search == false) {
        // If this is an inner for loop, skip it.
        if (ir_methods::is_loop(node)) {
            SgScopeStatement* scope_stmt = isSgScopeStatement(node);
            ROSE_ASSERT(scope_stmt);

            if (init_scope_stmt == NULL) {
                init_scope_stmt = scope_stmt;
            } else {
                // This for loop is inside an outer for loop.
                attr.skip = true;
                return attr;
            }
        }
    }

    if (!isSgLocatedNode(node)) {
        std::cerr << mprefix << "Debug info not present, cannot proceed!"
            << std::endl;
        return attr;
    }

    // Decide whether read / write depending on the operand of AssignOp
    if (parent && isSgAssignOp(parent)) {
        if (parent->getChildIndex(node) == 0) {
            attr.access_type = TYPE_WRITE;
        } else {
            attr.access_type = TYPE_READ;
        }
    }

    if (parent && (isSgAssignInitializer(parent) || isSgExprListExp(parent))) {
        attr.access_type = TYPE_READ;
    }

    if (parent && isSgCompoundAssignOp(parent)) {
        if (parent->getChildIndex(node) == 0) {
            attr.access_type = TYPE_READ_AND_WRITE;
        } else {
            attr.access_type = TYPE_READ;
        }
    }

    if (parent && (isSgPlusPlusOp(parent) || isSgMinusMinusOp(parent)) &&
            attr.access_type == TYPE_UNKNOWN) {
        attr.access_type = TYPE_READ_AND_WRITE;
    }

    if (parent && (isSgUnaryOp(parent) || isSgBinaryOp(parent)) &&
            attr.access_type == TYPE_UNKNOWN) {
        attr.access_type = TYPE_READ;
    }

    // LHS operand of PntrArrRefExp is always skipped
    if (parent && isSgPntrArrRefExp(parent) &&
            parent->getChildIndex(node) == 0) {
        attr.skip = true;
        return attr;
    }

    // RHS operand of PntrArrRefExp is always read and never written
    if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)
        attr.access_type = TYPE_READ;

    if (attr.access_type == TYPE_UNKNOWN || !isSgPntrArrRefExp(node) ||
            parent == NULL) {
        return attr;
    }

    // Is this node not representing the top-level dimension of an array or
    // is this array expression is used to read from another array?
    if (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) == 0) {
        return attr;
    }

#if 0
    // Are we sure whether this is a read or a write operation?
    if (attr.access_type != TYPE_UNKNOWN
            // Is this an array reference statement?
            && isSgPntrArrRefExp(node)
            && parent
            // Is this node representing the top-level dimension of an array or
            // is this array expression is used to read from another array?
            && (!isSgPntrArrRefExp(parent) ||
            (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0))) {
#endif
        std::string stream_name = ir_methods::strip_index_expr(
                node->unparseToString());

        SgLocatedNode* located_node = reinterpret_cast<SgLocatedNode*>(node);
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                     located_node->get_file_info()->get_filenameString());

        reference_info_t reference_info;
        reference_info.name = stream_name;
        reference_info.access_type = attr.access_type;
        reference_info.node = node;

        reference_list.push_back(reference_info);

        // Reset this attribute for any child expressions
        attr.skip = false;
#if 0
    }
#endif

    return attr;
}
