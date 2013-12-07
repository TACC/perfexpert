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
#include "instrumentor.h"
#include "ir_methods.h"
#include "macpo_record.h"
#include "streams.h"

using namespace SageBuilder;
using namespace SageInterface;

void instrumentor_t::atTraversalStart() {
    stream_list.clear();
    inst_info_list.clear();
}

void instrumentor_t::atTraversalEnd() {
    for (std::vector<inst_info_t>::iterator it=inst_info_list.begin();
            it!=inst_info_list.end(); it++)
        ir_methods::insert_instrumentation_call(*it);
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

    long count = 0;
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
        long ref_idx = reference_info.idx;

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
    }

    attr.skip = true;
    return attr;
}

const inst_list_t::iterator instrumentor_t::inst_begin() {
    return inst_info_list.begin();
}

const inst_list_t::iterator instrumentor_t::inst_end() {
    return inst_info_list.end();
}
