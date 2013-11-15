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

#include "instrumentor.h"
#include "ir_methods.h"
#include "macpo_record.h"

using namespace SageBuilder;
using namespace SageInterface;

void instrumentor_t::atTraversalStart() {
    reference_list.clear();
    inst_info_list.clear();
}

void instrumentor_t::atTraversalEnd() {
    for (std::vector<inst_info_t>::iterator it=inst_info_list.begin();
            it!=inst_info_list.end(); it++)
        ir_methods::insert_instrumentation_call(*it);
}

std::vector<reference_info_t>& instrumentor_t::get_reference_list() {
    return reference_list;
}

attrib instrumentor_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    // If explicit instructions to skip this node, then just return
    if (attr.skip)
        return attr;

    SgNode* parent = node->get_parent();

    // Decide whether read / write depending on the operand of AssignOp
    if (parent && isSgAssignOp(parent))
        attr.access_type = parent->getChildIndex(node) == 0 ? TYPE_WRITE : TYPE_READ;

    if (parent && isSgCompoundAssignOp(parent))
        attr.access_type = parent->getChildIndex(node) == 0 ? TYPE_READ_AND_WRITE : TYPE_READ;

    if (parent && (isSgPlusPlusOp(parent) || isSgMinusMinusOp(parent)) && attr.access_type == TYPE_UNKNOWN)
        attr.access_type = TYPE_READ_AND_WRITE;

    if (parent && (isSgUnaryOp(parent) || isSgBinaryOp(parent)) && attr.access_type == TYPE_UNKNOWN)
        attr.access_type = TYPE_READ;

    // LHS operand of PntrArrRefExp is always skipped
    if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) == 0) {
        attr.skip = true;
        return attr;
    }

    // RHS operand of PntrArrRefExp is always read and never written
    if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)
        attr.access_type = TYPE_READ;

    if (attr.access_type != TYPE_UNKNOWN	// Are we sure whether this is a read or a write operation?
            && isSgPntrArrRefExp(node)	// Is this an array reference statement?
            && parent
            && (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0))) { // Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
        if (!isSgLocatedNode(node))	{ // If no debug info present, can't do anything
            std::cerr << "Debug info not present, cannot proceed!" << std::endl;
            return attr;
        }

        SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(node);
        SgStatement* containingStmt = getEnclosingNode<SgStatement>(node);

        if (containingBB && containingStmt) {
            std::string stream_name = node->unparseToString();
            while (stream_name.at(stream_name.length()-1) == ']') {
                // Strip off the last [.*]
                unsigned index = stream_name.find_last_of('[');
                stream_name = stream_name.substr(0, index);
            }

            // For fortran array notation
            while (stream_name.at(stream_name.length()-1) == ')') {
                // Strip off the last [.*]
                unsigned index = stream_name.find_last_of('(');
                stream_name = stream_name.substr(0, index);
            }

            size_t idx = 0;
            for (std::vector<reference_info_t>::iterator it = reference_list.begin(); it != reference_list.end(); it++) {
                if (it->name == stream_name) {
                    break;
                }

                idx += 1;
            }

            if (idx == reference_list.size()) {
                reference_info_t stream_info;
                stream_info.name = stream_name;

                reference_list.push_back(stream_info);
                idx = reference_list.size()-1;
            }

            Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*) node)->get_file_info()->get_filenameString());

            int line_number=0;
            SgStatement *stmt = getEnclosingNode<SgStatement>(node);
            if (stmt)	line_number = stmt->get_file_info()->get_raw_line();

            // If not Fortran, cast the address to a void pointer
            SgExpression *param_addr = SageInterface::is_Fortran_language() ? (SgExpression*) node : buildCastExp (buildAddressOfOp((SgExpression*) node), buildPointerType(buildVoidType()));

            SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
            SgIntVal* param_idx = new SgIntVal(fileInfo, idx);
            SgIntVal* param_read_write = new SgIntVal(fileInfo, attr.access_type);

            inst_info_t inst_info;
            inst_info.bb = containingBB;
            inst_info.stmt = containingStmt;
            inst_info.params.push_back(param_read_write);
            inst_info.params.push_back(param_line_number);
            inst_info.params.push_back(param_addr);
            inst_info.params.push_back(param_idx);

            inst_info.function_name = SageInterface::is_Fortran_language() ? "indigo__record_f" : "indigo__record_c";
            inst_info.before = true;

            inst_info_list.push_back(inst_info);

            // Reset this attribute for any child expressions
            attr.skip = false;
        }
    }

    return attr;
}

const inst_list_t::iterator instrumentor_t::inst_begin() {
    return inst_info_list.begin();
}

const inst_list_t::iterator instrumentor_t::inst_end() {
    return inst_info_list.end();
}
