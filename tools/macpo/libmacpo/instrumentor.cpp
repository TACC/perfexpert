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
#include <string>
#include <vector>

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
    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        std::string stream = reference_info.name;

        if (count == reference_info.idx) {
            stream_list.push_back(stream);
            count += 1;
        }
    }

    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        reference_info_t& reference_info = *it;
        SgNode* ref_node = reference_info.node;
        std::string stream = reference_info.name;
        int16_t ref_access_type = reference_info.access_type;
        size_t ref_idx = reference_info.idx;

        SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(ref_node);
        SgStatement* containingStmt = getEnclosingNode<SgStatement>(ref_node);

        SgLocatedNode* located_ref = reinterpret_cast<SgLocatedNode*>(ref_node);
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    located_ref->get_file_info()->get_filenameString());

        int line_number = 0;
        SgStatement *stmt = getEnclosingNode<SgStatement>(ref_node);
        if (stmt) {
            line_number = stmt->get_file_info()->get_raw_line();
        }

        SgExpression* expr = isSgExpression(ref_node);
        ROSE_ASSERT(expr);

        // Strip unary operators like ++ or -- from the expression.
        SgExpression* stripped_expr = NULL;
        stripped_expr = ir_methods::strip_unary_operators(expr);
        ROSE_ASSERT(stripped_expr && "Bug in stripping unary operators "
                "from given expression!");

        // If not Fortran, cast the address to a void pointer
        SgExpression *param_addr = SageInterface::is_Fortran_language() ?
            stripped_expr : buildCastExp(
                    buildAddressOfOp(stripped_expr),
                    buildPointerType(buildVoidType()));

        SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
        SgIntVal* param_idx = new SgIntVal(fileInfo, ref_idx);
        SgIntVal* param_read_write = new SgIntVal(fileInfo, ref_access_type);

        param_line_number->set_endOfConstruct(fileInfo);
        param_idx->set_endOfConstruct(fileInfo);
        param_read_write->set_endOfConstruct(fileInfo);
        std::string function_name = "";
        if (use_dyanmic_inst) {
            function_name = SageInterface::is_Fortran_language() ?
                "indigo__process_f" : "indigo__process_c";
        } else {
            function_name = SageInterface::is_Fortran_language() ?
                "indigo__record_f" : "indigo__record_c";
        }
    

        SgType* type = expr->get_type();
        SgSizeOfOp* size_of_op;
        SgExpression *storage_size_op;
        if (!SageInterface::is_Fortran_language()) {
            size_of_op = new SgSizeOfOp(fileInfo, NULL, type, type);
            size_of_op->set_endOfConstruct(fileInfo);
            ROSE_ASSERT(size_of_op);
        } else {
            // SgSizeofOp doesn't work for fortran.
            // storage_size gives the size of one element of given array.
            std::vector<SgExpression*> storage_size_params;
            SgVarRefExp *ref_param = buildVarRefExp(SgName(stream), containingBB);
            storage_size_params.push_back(ref_param);

            SgExprListExp* storage_size_params_list =  SageBuilder::buildExprListExp(storage_size_params);
            storage_size_op = SageBuilder::buildFunctionCallExp ("storage_size", type, storage_size_params_list, containingBB);
        }


        std::vector<SgExpression*> params;
        params.push_back(param_read_write);
        params.push_back(param_line_number);
        params.push_back(param_addr);
        params.push_back(param_idx);

        if (SageInterface::is_Fortran_language()) {
            params.push_back(storage_size_op);
        } else {
            params.push_back(size_of_op);
        }

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

void instrumentor_t::set_dynamic_instrumentation(bool dyanmic_inst) {
    use_dyanmic_inst = dyanmic_inst;
}
