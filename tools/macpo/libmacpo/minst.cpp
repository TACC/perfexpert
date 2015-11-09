/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#include <rose.h>

#include <string>
#include <vector>

#include "aligncheck.h"
#include "analysis_profile.h"
#include "argparse.h"
#include "branchpath.h"
#include "inst_defs.h"
#include "instrumentor.h"
#include "ir_methods.h"
#include "loop_traversal.h"
#include "minst.h"
#include "pntr_overlap.h"
#include "reuse_dist.h"
#include "stride_check.h"
#include "tracer.h"
#include "tripcount.h"
#include "vector_strides.h"

using namespace SageBuilder;
using namespace SageInterface;

bool midend(SgProject* project, options_t& options) {
    // Check if we really need to invoke the instrumentation.
    if (options.location_list.size() == 0) {
        return true;
    }

    // uncomment to generate a .dot graph
    // generateDOT(*project);

    SgFilePtrList files = project->get_fileList();
    if (options.backup_filename.size()) {
        // We need to save the input file to a backup file.
        if (files.size() != 1) {
            std::cerr << macpoprefix << "Backup option can be specified with only "
                << "a single file for compilation, terminating." << std::endl;
            return false;
        }

        SgSourceFile* file = isSgSourceFile(*(files.begin()));
        std::string source = file->get_file_info()->get_filenameString();

        // Copy the file over.
        if (argparse::copy_file(source.c_str(),
                options.backup_filename.c_str()) < 0) {
            std::cerr << macpoprefix << "Error backing up file." << std::endl;
            return false;
        }

        std::cerr << macpoprefix << "Saved " << source << " into " <<
            options.backup_filename << "." << std::endl;
    }

    // Loop over each file
    for (SgFilePtrList::iterator it = files.begin(); it != files.end(); it++) {
        SgSourceFile* file = isSgSourceFile(*it);

        // Start the traversal!
        MINST traversal(options, project);
        traversal.traverseWithinFile(file, preorder);
    }

    return true;
}

MINST::MINST(const options_t& _options, SgProject* project)
    : options(_options) {
    VariableRenaming* var_renaming = new VariableRenaming(project);
    def_table = var_renaming->getDefTable();
}

void MINST::insert_map_prototype(SgNode* node) {
    const char* func_name = "indigo__create_map";

    if (!SageInterface::is_Fortran_language()) {
        SgFunctionDeclaration *decl;
        decl = SageBuilder::buildDefiningFunctionDeclaration(func_name,
                buildVoidType(), buildFunctionParameterList(), global_node);
        non_def_decl = SageBuilder::buildNondefiningFunctionDeclaration(decl,
                global_node);
        non_def_decl->get_functionModifier().setGnuAttributeWeak();
        //SgLinkageModifier* link(non_def_decl->get_freepointer());
        //link->setCppLinkage();
        //((SgLinkageModifier* )(global_node))->setCppLinkage ();

    }
}

void MINST::insert_map_function(SgNode* node) {
    if (map_function_inserted == false) {
        SgProcedureHeaderStatement* header;
        const char* func_name = "indigo__create_map";

        if (SageInterface::is_Fortran_language()) {
            header = SageBuilder::buildProcedureHeaderStatement(func_name,
                    buildVoidType(), buildFunctionParameterList(),
                    SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
                    global_node);
            def_decl = isSgFunctionDeclaration(header);
        } else {
            def_decl = SageBuilder::buildDefiningFunctionDeclaration(func_name,
                    buildVoidType(), buildFunctionParameterList(), global_node);
        }

        appendStatement(def_decl, global_node);

        map_function_inserted = true;
    }
}

void MINST::atTraversalStart() {
    statement_list.clear();
    stream_list.clear();
    file_list.clear();

    map_function_inserted = false;
    global_node = NULL;
    non_def_decl = NULL;
    def_decl = NULL;
}

void MINST::atTraversalEnd() {
    if (global_node && non_def_decl)
        global_node->prepend_declaration(non_def_decl);

    if (def_decl) {
        SgBasicBlock* bb = def_decl->get_definition()->get_body();
        SgLocatedNode* located_bb = reinterpret_cast<SgLocatedNode*>(bb);
        Sg_File_Info* file_info = located_bb->get_file_info();

        if (bb && stream_list.size() > 0) {
            std::string indigo__write_idx;
            if (SageInterface::is_Fortran_language()) {
                indigo__write_idx = "indigo__write_idx_f";
            } else {
                indigo__write_idx = "indigo__write_idx_c";
            }

            for (name_list_t::iterator it = stream_list.begin();
                    it != stream_list.end(); it++) {
                // Add a call to indigo__write_idx_[fc]()
                // and place it in the basic block bb.
                std::string stream_name = *it;

                std::vector<SgExpression*> expr_vector;
                SgStringVal* param_stream_name = new SgStringVal(file_info,
                        stream_name);
                param_stream_name->set_endOfConstruct(file_info);
                expr_vector.push_back(param_stream_name);

                SgIntVal* param_length = new SgIntVal(file_info,
                        stream_name.size());
                param_length->set_endOfConstruct(file_info);
                expr_vector.push_back(param_length);

                SgExprStatement* write_idx_call =
                    buildFunctionCallStmt(SgName(indigo__write_idx),
                            buildVoidType(), buildExprListExp(expr_vector), bb);
                write_idx_call->set_parent(bb);
                bb->append_statement(write_idx_call);
            }
        }
    }

    // Add instrumentation statements as requested.
    for (statement_list_t::iterator it = statement_list.begin();
            it != statement_list.end(); it++) {
        statement_info_t& statement_info = *it;

        SgStatement* ref_stmt = statement_info.reference_statement;
        SgStatement* stmt = statement_info.statement;

        ir_methods::match_end_of_constructs(ref_stmt, stmt);
        if (statement_info.before) {
            insertStatementBefore(ref_stmt, stmt);
        } else {
            insertStatementAfter(ref_stmt, stmt);
        }
    }
}

const analysis_profile_list MINST::run_analysis(SgNode* node, int16_t action) {
    std::vector<analysis_profile_t> profile_list;
    if (is_action(action, ACTION_INSTRUMENT)) {
        insert_map_function(node);

        instrumentor_t inst;
        inst.set_dynamic_instrumentation(options.dynamic_inst);
        inst.traverse(node, attrib());

        // Pull information from AST traversal.
        stream_list = inst.get_stream_list();
        statement_list.insert(statement_list.end(), inst.stmt_begin(),
                inst.stmt_end());

        profile_list.push_back(inst.get_analysis_profile());
    }

    if (is_action(action, ACTION_ALIGNCHECK)) {
        aligncheck_t visitor(def_table);
        visitor.process_node(node);

        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_TRIPCOUNT)) {
        tripcount_t visitor(def_table);
        visitor.process_node(node);

        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_BRANCHPATH)) {
        branchpath_t visitor(def_table);
        visitor.process_node(node);

        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_GENTRACE)) {
        insert_map_function(node);

        tracer_t tracer;
        tracer.traverse(node, attrib());

        // Pull information from AST traversal.
        stream_list = tracer.get_stream_list();
        statement_list.insert(statement_list.end(), tracer.stmt_begin(),
                tracer.stmt_end());

        profile_list.push_back(tracer.get_analysis_profile());
    }

    if (is_action(action, ACTION_VECTORSTRIDES)) {
        insert_map_function(node);

        vector_strides_t visitor(def_table);
        visitor.process_node(node);

        stream_list = visitor.get_stream_list();
        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_OVERLAPCHECK)) {
        pntr_overlap_t visitor(def_table);
        visitor.process_node(node);

        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_STRIDECHECK)) {
        stride_check_t visitor(def_table);
        visitor.process_node(node);

        stream_list = visitor.get_stream_list();
        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    if (is_action(action, ACTION_REUSEDISTANCE)) {
        insert_map_function(node);

        reuse_dist_t visitor(def_table);
        visitor.process_node(node);

        stream_list = visitor.get_stream_list();
        statement_list.insert(statement_list.end(), visitor.stmt_begin(),
                visitor.stmt_end());

        profile_list.push_back(visitor.get_analysis_profile());
    }

    return profile_list;
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
        std::cerr << macpoprefix << "Processed loop at " << file_name << ":" <<
            line_number << "." << std::endl;
    } else {
        std::cerr << macpoprefix << "Unsupported loop at " << file_name << ":" <<
            line_number << "." << std::endl;
    }

    for (std::vector<loop_info_list_t>::const_iterator it =
            loop_info.child_loop_info.begin();
            it != loop_info.child_loop_info.end(); it++) {
        const loop_info_list_t& loop_info_list = *it;
        for (loop_info_list_t::const_iterator it2 = loop_info_list.begin();
                it2 != loop_info_list.end(); it2++) {
            const loop_info_t& loop_info = *it2;

            print_loop_processing_status(loop_info);
        }
    }
}

void MINST::analyze_node(SgNode* node, int16_t action) {
    size_t last_statement_count = statement_list.size();

    SgLocatedNode* located_node = reinterpret_cast<SgLocatedNode*>(node);
    Sg_File_Info* file_info = located_node->get_file_info();
    const std::string& file_name = file_info->get_filenameString();
    int line_number = file_info->get_line();

    const analysis_profile_list& profile_list = run_analysis(node, action);

    if (options.profile_analysis) {
        double analysis_time = 0;
        for (analysis_profile_list::const_iterator it = profile_list.begin();
                it != profile_list.end(); it++) {
            const analysis_profile_t& profile = *it;
            const loop_info_list_t& loop_info_list =
                profile.get_loop_info_list();

            for (loop_info_list_t::const_iterator it = loop_info_list.begin();
                    it != loop_info_list.end(); it++) {
                const loop_info_t& loop_info = *it;
                print_loop_processing_status(loop_info);
            }

            analysis_time += profile.get_running_time();
        }

        std::cerr << macpoprefix << "Analysis time: " << analysis_time <<
            " second(s)." << std::endl;
    }

    if (statement_list.size() != last_statement_count) {
        std::cerr << macpoprefix << "Analyzed code at " << file_name << ":" <<
            line_number << "." << std::endl;
    }
}

void MINST::add_hooks_to_main_function(SgFunctionDefinition* main_def) {
    // Add header file for indigo's record function
    ROSE_ASSERT(global_node);
    if (!SageInterface::is_Fortran_language()) {
        insertHeader("mrt.h", PreprocessingInfo::after, false, global_node);

        if (options.dynamic_inst) {
            insertHeader("set_cache_conflict_lib.h", PreprocessingInfo::after, false,
                            global_node);
        }
    }

    SgLocatedNode* located_node = reinterpret_cast<SgLocatedNode*>(main_def);
    Sg_File_Info* file_info = located_node->get_file_info();

    // Insert calls to indigo__init() and indigo__create_map().
    ROSE_ASSERT(main_def);
    SgBasicBlock* body = main_def->get_body();

    // Skip over the initial variable declarations and `IMPLICIT' statements.
    SgStatement *statement = NULL;
    SgStatementPtrList& stmts = body->get_statements();
    for (SgStatementPtrList::iterator it = stmts.begin(); it != stmts.end();
            it++) {
        statement = *it;

        if (!isSgImplicitStatement(statement) &&
                !isSgDeclarationStatement(statement)) {
            break;
        }
    }

    SgStatement *last_stmt = NULL;
    SgStatementPtrList::iterator it = stmts.end();
    --it; // skip the null
    for (; it != stmts.begin();
            it--) {
        last_stmt = *it;

        if(reinterpret_cast<SgReturnStmt*>(last_stmt)) {
            break;
        }
        it--;
    }

    ROSE_ASSERT(last_stmt);

    if (statement == NULL) {
        return;
    }

    std::string indigo__create_map = "indigo__create_map";
    std::string  indigo__init;
    if (SageInterface::is_Fortran_language()) {
        indigo__init = "indigo__init";
    } else {
        indigo__init = "indigo__init_";
    }

    int create_file = 0;
    int enable_sampling;

    bool insert_map_call = false;
    std::vector<SgExpression*> params, empty_params;

    for (location_list_t::const_iterator it = options.location_list.begin();
            it != options.location_list.end(); it++) {
        location_t location = *it;
        if (create_file == 0 &&
            (is_action(location.action, ACTION_INSTRUMENT) ||
                is_action(location.action, ACTION_VECTORSTRIDES))) {
            create_file = 1;
        }

        if (insert_map_call == false &&
            (is_action(location.action, ACTION_INSTRUMENT) ||
            is_action(location.action, ACTION_VECTORSTRIDES) ||
            is_action(location.action, ACTION_REUSEDISTANCE))) {
            insert_map_call = true;
        }

        if (create_file == 1 && insert_map_call == true) {
            break;
        }
    }

    enable_sampling = options.disable_sampling ? 0 : 1;

    SgIntVal* rose_create_file = new SgIntVal(file_info, create_file);
    rose_create_file->set_endOfConstruct(file_info);
    params.push_back(rose_create_file);

    SgIntVal* rose_enable_sampling = new SgIntVal(file_info, enable_sampling);
    rose_enable_sampling->set_endOfConstruct(file_info);
    params.push_back(rose_enable_sampling);

    SgExprStatement* expr_stmt = NULL;
    expr_stmt = ir_methods::prepare_call_statement(body, indigo__init, params,
            statement);
    insertStatementBefore(statement, expr_stmt);
    ROSE_ASSERT(expr_stmt);

    if (options.dynamic_inst == true) {
        SgExprStatement* end_stmt = NULL;
        end_stmt = ir_methods::prepare_call_statement(body, "indigo__end", empty_params,
                last_stmt);
        insertStatementBefore(last_stmt, end_stmt);
        ROSE_ASSERT(end_stmt);
    }

    if (insert_map_call) {
        SgExprStatement* map_stmt = NULL;
        map_stmt = ir_methods::prepare_call_statement(body,
                indigo__create_map, empty_params, expr_stmt);
        insertStatementAfter(expr_stmt, map_stmt);
        ROSE_ASSERT(map_stmt);

        insert_map_prototype(main_def);
    }
}

void MINST::visit(SgNode* node) {
    if (!isSgLocatedNode(node)) {
        // Cannot determine the line number corresponding to this node.
        return;
    }

    if (isSgGlobal(node)) {
        global_node = static_cast<SgGlobal*>(node);
    }

    bool is_loop = ir_methods::is_loop(node);
    bool is_function = ir_methods::is_function(node);
    if (is_loop == false && is_function == false) {
        return;
    }

    SgLocatedNode* located_node = reinterpret_cast<SgLocatedNode*>(node);
    Sg_File_Info* file_info = located_node->get_file_info();
    const std::string& _this_file_name = file_info->get_filenameString();
    int _this_line_number = file_info->get_line();

    bool main_def = false;
    SgFunctionDefinition* def_node = isSgFunctionDefinition(node);

    
    if (SageInterface::is_Fortran_language()) {
        std::cout << "It is Fortan!!" << std::endl;
        SgNode *parent = node->get_parent();
        SgProgramHeaderStatement *program_node = isSgProgramHeaderStatement(parent);
        if (program_node) {
            std::cout << "Main node found" << std::endl;
            main_def = true;
        }
    }

    if (def_node) {
        SgFunctionDeclaration* decl_node = def_node->get_declaration();
        if (decl_node->get_name().getString() == "main") {
            main_def = true;
        }
    } else {
        def_node = getEnclosingNode<SgFunctionDefinition>(node);
    }

    // If still NULL, we don't know what else to do.
    if (def_node == NULL) {
        return;
    }

    SgFunctionDeclaration* decl_node = def_node->get_declaration();
    std::string _this_func_name = decl_node->get_name();

    // Check if this is the 'main()' function.
    if (main_def == true) {
        add_hooks_to_main_function(def_node);
    }

    if (options.location_list.size() == 0) {
        return;
    }

    int16_t action = options.get_action("<all>");
    action |= options.get_action(_this_func_name, _this_line_number);
    action |= options.get_action(_this_file_name, _this_line_number);

    if (is_function) {
        action |= options.get_action(_this_func_name);
        action |= options.get_action(_this_file_name);
    }

    // Validate the selected action.
    if (action <= ACTION_NONE || action >= ACTION_LAST) {
        return;
    }

    // Add header file for indigo's record function.
    ROSE_ASSERT(global_node);

    // If this file hasn't been encountered in the past,
    // then add the mrt.h header include line.
    name_list_t::iterator st = file_list.begin();
    name_list_t::iterator en = file_list.end();
    if (std::find(st, en, _this_file_name) != en) {
        if (!SageInterface::is_Fortran_language()) {
            insertHeader("mrt.h", PreprocessingInfo::after, false,
                    global_node);
            if (options.dynamic_inst) {
                insertHeader("set_cache_conflict_lib.h", PreprocessingInfo::after, false,
                        global_node);
            }
        }

        file_list.push_back(_this_file_name);
    }

    analyze_node(node, action);
}
