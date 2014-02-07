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

#include <cstdlib>
#include <rose.h>

#include "aligncheck.h"
#include "analysis_profile.h"
#include "inst_defs.h"
#include "instrumentor.h"
#include "ir_methods.h"
#include "loop_traversal.h"
#include "minst.h"
#include "tracer.h"
#include "vector_strides.h"

using namespace SageBuilder;
using namespace SageInterface;

MINST::MINST(short _action, int _line_number, std::string _inst_func,
        bool _profile_analysis, VariableRenaming* _var_renaming) {
    action = _action;
    line_number = _line_number;
    inst_func = _inst_func;
    profile_analysis = _profile_analysis;
    var_renaming = _var_renaming;
}

void MINST::insert_map_prototype(SgNode* node) {
    const char* func_name = "indigo__create_map";

    if (!SageInterface::is_Fortran_language()) {
        SgFunctionDeclaration *decl;
        decl = SageBuilder::buildDefiningFunctionDeclaration(func_name, buildVoidType(), buildFunctionParameterList(), global_node);
        non_def_decl = SageBuilder::buildNondefiningFunctionDeclaration(decl, global_node);
    }
}

void MINST::insert_map_function(SgNode* node) {
    SgProcedureHeaderStatement* header;
    const char* func_name = "indigo__create_map";

    if (SageInterface::is_Fortran_language()) {
        header = SageBuilder::buildProcedureHeaderStatement(func_name, buildVoidType(), buildFunctionParameterList(), SgProcedureHeaderStatement::e_subroutine_subprogram_kind, global_node);
        def_decl = isSgFunctionDeclaration(header);
    } else {
        def_decl = SageBuilder::buildDefiningFunctionDeclaration(func_name, buildVoidType(), buildFunctionParameterList(), global_node);
    }

    appendStatement(def_decl, global_node);
}

void MINST::atTraversalStart() {
    statement_list.clear();
    stream_list.clear();
    global_node=NULL, non_def_decl=NULL, def_decl=NULL, file_info=NULL;
}

void MINST::atTraversalEnd() {
    if (global_node && non_def_decl)
        global_node->prepend_declaration(non_def_decl);

    if (def_decl) {
        SgBasicBlock* bb = def_decl->get_definition()->get_body();
        if (bb && stream_list.size() > 0) {
            std::string indigo__write_idx = SageInterface::is_Fortran_language() ? "indigo__write_idx_f" : "indigo__write_idx_c";
            for (name_list_t::iterator it = stream_list.begin(); it != stream_list.end(); it++) {
                // Add a call to indigo__write_idx_[fc]() and place it in the basic block bb
                std::string stream_name = *it;

                std::vector<SgExpression*> expr_vector;
                SgStringVal* param_stream_name = new SgStringVal(file_info, stream_name);
                expr_vector.push_back(param_stream_name);

                SgIntVal* param_length = new SgIntVal(file_info, stream_name.size());
                expr_vector.push_back(param_length);

                SgExprStatement* write_idx_call = buildFunctionCallStmt(SgName(indigo__write_idx), buildVoidType(), buildExprListExp(expr_vector), bb);
                bb->append_statement(write_idx_call);
            }
        }
    }

    // Add instrumentation statements as requested.
    for (statement_list_t::iterator it = statement_list.begin();
            it != statement_list.end(); it++) {
        statement_info_t& statement_info = *it;

        if (statement_info.before) {
            insertStatementBefore(statement_info.reference_statement,
                    statement_info.statement);
        } else {
            insertStatementAfter(statement_info.reference_statement,
                    statement_info.statement);
        }
    }
}

bool MINST::is_same_file(const std::string& file_1, const std::string& file_2) {
    char path_1[PATH_MAX+1], path_2[PATH_MAX+1];

    std::string canonical_file_1 = (file_1[0] != '/' && file_1[0] != '.' ? "./"
            : "") + file_1;

    std::string canonical_file_2 = (file_2[0] != '/' && file_2[0] != '.' ? "./"
            : "") + file_2;

    if (realpath(canonical_file_1.c_str(), path_1) == NULL) {
        return false;
    }

    if (realpath(canonical_file_2.c_str(), path_2) == NULL) {
        return false;
    }

    return strcmp(path_1, path_2) == 0;
}

const analysis_profile_t MINST::run_analysis(SgNode* node, short action) {
    switch(action) {
        case ACTION_INSTRUMENT:
            {
                insert_map_function(node);

                instrumentor_t inst;
                inst.traverse(node, attrib());

                // Pull information from AST traversal.
                stream_list = inst.get_stream_list();
                statement_list.insert(statement_list.end(),
                        inst.stmt_begin(), inst.stmt_end());

                return inst.get_analysis_profile();
            }

        case ACTION_ALIGNCHECK:
            {
                aligncheck_t visitor(var_renaming);
                visitor.process_node(node);

                stream_list = visitor.get_stream_list();
                statement_list.insert(statement_list.end(),
                        visitor.stmt_begin(), visitor.stmt_end());

                return visitor.get_analysis_profile();
            }

        case ACTION_GENTRACE:
            {
                insert_map_function(node);

                tracer_t tracer;
                tracer.traverse(node, attrib());

                // Pull information from AST traversal.
                stream_list = tracer.get_stream_list();
                statement_list.insert(statement_list.end(),
                        tracer.stmt_begin(), tracer.stmt_end());

                return tracer.get_analysis_profile();
            }

        case ACTION_VECTORSTRIDES:
            {
                insert_map_function(node);

                vector_strides_t visitor(var_renaming);
                visitor.process_node(node);

                stream_list = visitor.get_stream_list();
                statement_list.insert(statement_list.end(),
                        visitor.stmt_begin(), visitor.stmt_end());

                return visitor.get_analysis_profile();
            }
    }

    ROSE_ASSERT(false && "Invalid action!");
}

void MINST::print_loop_processing_status(const loop_info_t& loop_info) {
    SgNode* loop_node = loop_info.loop_stmt;
    SgLocatedNode* located_node = isSgLocatedNode(loop_node);

    ROSE_ASSERT(located_node && "Failed to fetch line number information for "
            "loop.");

    Sg_File_Info* file_info = NULL;
    file_info = located_node->get_file_info();

    const std::string& file_name = file_info->get_filenameString();
    int line_number = file_info->get_line();

    if (loop_info.processed) {
        std::cerr << mprefix << "Processed loop at " << file_name << ":" <<
            line_number << "." << std::endl;
    } else {
        std::cerr << mprefix << "Unsupported loop at " << file_name << ":" <<
            line_number << "." << std::endl;
    }

    for(std::vector<loop_info_list_t>::const_iterator it =
            loop_info.child_loop_info.begin();
            it != loop_info.child_loop_info.end(); it++) {
        const loop_info_list_t& loop_info_list = *it;
        for(loop_info_list_t::const_iterator it2 = loop_info_list.begin();
                it2 != loop_info_list.end(); it2++) {
            const loop_info_t& loop_info = *it2;

            print_loop_processing_status(loop_info);
        }
    }
}

void MINST::analyze_node(SgNode* node, short action) {
    size_t last_statement_count = statement_list.size();

    Sg_File_Info* file_info = ((SgLocatedNode *) node)->get_file_info();
    const std::string& file_name = file_info->get_filenameString();
    int line_number = file_info->get_line();

    const analysis_profile_t& profile = run_analysis(node, action);

    if (profile_analysis) {
        const loop_info_list_t& loop_info_list = profile.get_loop_info_list();

        for(loop_info_list_t::const_iterator it = loop_info_list.begin();
                it != loop_info_list.end(); it++) {
            const loop_info_t& loop_info = *it;
            print_loop_processing_status(loop_info);
        }

        const double analysis_time =  profile.get_running_time();
        std::cerr << mprefix << "Analysis time: " << analysis_time <<
            " second(s)." << std::endl;
    }

    if (statement_list.size() != last_statement_count) {
        std::cerr << mprefix << "Analyzed code at " << file_name << ":" <<
            line_number << "." << std::endl;
    } else {
        // No change in statement count.
        std::cerr << mprefix << "Code at " << file_name << ":" << line_number <<
            " could not be analyzed because of unsupported loop structure(s)."
            << std::endl;
    }
}

void MINST::visit(SgNode* node)
{
    // Add header file for indigo's record function
    if (isSgGlobal(node)) {
        global_node = static_cast<SgGlobal*>(node);
        file_info = Sg_File_Info::generateFileInfoForTransformationNode(
                ((SgLocatedNode*) node)->get_file_info()->get_filenameString());

        if (!SageInterface::is_Fortran_language())
            insertHeader("mrt.h", PreprocessingInfo::after, false, global_node);
    }

    if (!isSgLocatedNode(node)) {
        // Cannot determine the line number corresponding to this node.
        return;
    }

    Sg_File_Info* file_info = ((SgLocatedNode *) node)->get_file_info();
    const std::string& _file_name = file_info->get_filenameString();
    int _line_number = file_info->get_line();

    // Check if this is the function that we are told to instrument
    if (isSgFunctionDefinition(node)) {
        std::string function_name = ((SgFunctionDefinition*) node)->get_declaration()->get_name();
        if (function_name == "main") {
            // Found main, now insert calls to indigo__init() and indigo__create_map()
            SgBasicBlock* body = ((SgFunctionDefinition*) node)->get_body();

            // Skip over the initial variable declarations and `IMPLICIT' statements
            SgStatement *statement=NULL;
            SgStatementPtrList& stmts = body->get_statements();
            for (SgStatementPtrList::iterator it=stmts.begin(); it!=stmts.end(); it++) {
                statement=*it;

                if (!isSgImplicitStatement(statement) && !isSgDeclarationStatement(statement))
                    break;
            }

            if(statement == NULL)
                return;

            std::string indigo__init = SageInterface::is_Fortran_language() ? "indigo__init" : "indigo__init_";
            std::string indigo__create_map = "indigo__create_map";

            std::vector<SgExpression*> params, empty_params;
            int create_file, enable_sampling;

            if (action == ACTION_ALIGNCHECK) {
                create_file = 0;
            } else {
                create_file = 1;
            }

            if (action == ACTION_GENTRACE) {
                enable_sampling = 0;   // Disable sampling
            } else {
                enable_sampling = 1;
            }

            SgIntVal* rose_create_file = new SgIntVal(file_info, create_file);
            rose_create_file->set_endOfConstruct(file_info);
            params.push_back(rose_create_file);

            SgIntVal* rose_enable_sampling = new SgIntVal(file_info,
                    enable_sampling);
            rose_enable_sampling->set_endOfConstruct(file_info);
            params.push_back(rose_enable_sampling);

            SgExprStatement* expr_stmt = NULL;
            expr_stmt = ir_methods::prepare_call_statement(body, indigo__init,
                    params, statement);
            insertStatementBefore(statement, expr_stmt);
            ROSE_ASSERT(expr_stmt);

            if (action != ACTION_ALIGNCHECK) {
                SgExprStatement* map_stmt = NULL;
                map_stmt = ir_methods::prepare_call_statement(body,
                        indigo__create_map, empty_params, expr_stmt);
                insertStatementAfter(expr_stmt, map_stmt);
                ROSE_ASSERT(map_stmt);

                insert_map_prototype(node);
            }
        }

        if (line_number == 0) {
            if (function_name != inst_func &&
                    is_same_file(_file_name, inst_func) == false)
                return;

            analyze_node(node, action);
        }
    }
    else if (line_number != 0)	{
        // We have to instrument some loops
        if (_line_number == line_number && ir_methods::is_loop(node)) {
            SgFunctionDefinition* def =
                getEnclosingNode<SgFunctionDefinition>(node);
            std::string function_name = def->get_declaration()->get_name();

            if (function_name != inst_func &&
                    is_same_file(_file_name, inst_func) == false)
                return;

            analyze_node(node, action);
        }
    }
}
