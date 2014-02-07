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

#include "analysis_profile.h"
#include "inst_defs.h"
#include "instrumentor.h"
#include "ir_methods.h"
#include "macpo_record.h"
#include "streams.h"

using namespace SageBuilder;
using namespace SageInterface;

void instrumentor_t::atTraversalStart() {
    analysis_profile.start_timer();
    stream_list.clear();
    statement_list.clear();
}

void instrumentor_t::atTraversalEnd() {
    analysis_profile.end_timer();
}

name_list_t& instrumentor_t::get_stream_list() {
    return stream_list;
}

attrib instrumentor_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    if (attr.skip)
        return attr;

    streams_t streams;
    streams.traverse(node, attrib());

    reference_list_t& reference_list = streams.get_reference_list();

    size_t count = 0;
    for(reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        std::string stream = reference_info.name;

        if (count == reference_info.idx) {
            stream_list.push_back(stream);
            count += 1;
        }
    }

    for(reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        SgNode* ref_node = reference_info.node;
        std::string stream = reference_info.name;
        short ref_access_type = reference_info.access_type;
        size_t ref_idx = reference_info.idx;

        SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(ref_node);
        SgStatement* containingStmt = getEnclosingNode<SgStatement>(ref_node);

        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*)
                     ref_node)->get_file_info()->get_filenameString());

        int line_number = 0;
        SgStatement *stmt = getEnclosingNode<SgStatement>(ref_node);
        if (stmt)	line_number = stmt->get_file_info()->get_raw_line();

        // If not Fortran, cast the address to a void pointer
        SgExpression *param_addr = SageInterface::is_Fortran_language() ?
            (SgExpression*) ref_node : buildCastExp (
                    buildAddressOfOp((SgExpression*) ref_node),
                    buildPointerType(buildVoidType()));

        SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
        SgIntVal* param_idx = new SgIntVal(fileInfo, ref_idx);
        SgIntVal* param_read_write = new SgIntVal(fileInfo, ref_access_type);

        std::string function_name = SageInterface::is_Fortran_language() ?
                "indigo__record_f" : "indigo__record_c";

        ROSE_ASSERT(isSgExpression(ref_node));
        SgExpression* expr = isSgExpression(ref_node);
        SgType* type = expr->get_type();
        SgSizeOfOp* size_of_op = new SgSizeOfOp(fileInfo, NULL, type, type);

        ROSE_ASSERT(size_of_op);

        std::vector<SgExpression*> params;
        params.push_back(param_read_write);
        params.push_back(param_line_number);
        params.push_back(param_addr);
        params.push_back(param_idx);
        params.push_back(size_of_op);

        statement_info_t statement_info;
        statement_info.statement = ir_methods::prepare_call_statement(
                containingBB, function_name, params, containingStmt);
        statement_info.reference_statement = containingStmt;
        statement_info.before = true;
        statement_list.push_back(statement_info);
    }

    attr.skip = true;
    return attr;
}

const analysis_profile_t& instrumentor_t::get_analysis_profile() {
    return analysis_profile;
}

const statement_list_t::iterator instrumentor_t::stmt_begin() {
    return statement_list.begin();
}

const statement_list_t::iterator instrumentor_t::stmt_end() {
    return statement_list.end();
}
